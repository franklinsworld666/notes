# 从零理解 NVMe-MI


## 先看全景：本文要解决的问题

假设一块 M.2 NVMe SSD 装在服务器中。主机操作系统通过 PCIe 使用它存数据；而 BMC 即使主机没启动，也希望取得 SSD 温度、健康状态和序列号。BMC 通常并不在这块 SSD 的 PCIe 数据路径上，因此它需要一条**管理路径**。

本文示例使用下面的硬件拓扑：

```text
                         主机 CPU
                            │
                    PCIe（数据读写）
                            │
                         M.2 SSD

 BMC
  │
  │ I2C/SMBus：管理通道
  ▼
 bus4 ── PCA9544 I2C MUX ──┬── bus60 ── M.2 SSD #0
                           ├── bus61 ── M.2 SSD #1
                           ├── bus62 ── M.2 SSD #2
                           └── bus63 ── M.2 SSD #3

            这条管理通道上可承载 MCTP，再承载 NVMe-MI
```

其中 `bus4`、`bus60` 到 `bus63` 是 Linux/OpenBMC 中常见的**逻辑 I2C 总线编号**。它们是软件编号，不是协议地址，也不是 MCTP EID。

---

## 1. 为什么需要 NVMe-MI

### 1.1 先凭直觉理解

把 SSD 想成一栋楼：主机操作系统走正门（PCIe）进楼，读写数据；BMC 是楼宇管理员，即使正门关着、主机宕机或操作系统还没起来，也要能从管理门了解楼里的温度和状态。

普通 NVMe 是给主机走“正门”用的。NVMe-MI（NVMe Management Interface）则是给 BMC、管理控制器等走“管理门”用的。

```text
主机 OS ── PCIe ── 普通 NVMe 命令 ── SSD
                  用于：读写数据、创建队列、管理命名空间

BMC ── SMBus / PCIe VDM ── MCTP ── NVMe-MI ── SSD
                  用于：识别、健康监测、温度、管理操作
```

### 1.2 为什么不能直接使用 `/dev/nvme0`

`/dev/nvme0` 是主机 OS 枚举 PCIe NVMe 控制器后创建的设备节点。BMC 通常运行着另一套 Linux，且与主机 CPU 相互独立：

```text
BMC Linux                       Host Linux
─────────                       ──────────
没有 /dev/nvme0        ≠        有 /dev/nvme0
连接到管理总线                  连接到 PCIe Root Complex
```

因此，BMC 要管理 SSD，不能假定主机已启动或主机 NVMe 驱动已经工作。NVMe-MI 定义了一个独立于主机数据路径的带外管理接口。

### 1.3 In-Band 与 Out-of-Band

- **In-Band Management（带内管理）**：管理命令走主机正常使用的 PCIe/NVMe 路径。例如主机上的 `nvme` 工具读取 SMART 日志。
- **Out-of-Band Management（带外管理）**：管理命令走独立于主机数据路径的通道。例如 BMC 经 SMBus + MCTP 与 SSD 通信。

带外管理的重要价值是：即使主机关机、卡死，或还未加载 NVMe 驱动，BMC 仍可能取得硬件健康信息。

### 1.4 正式定义

NVMe-MI 是 NVM Express 组织定义的管理接口规范。它规定了管理方与 NVMe 设备之间的消息、命令和数据格式；它**不等于一根物理总线**。NVMe-MI 可以借助 MCTP 在 SMBus/I2C 等带外通道上传输，也可以借助 PCIe VDM（Vendor Defined Message）传输。

---

## 2. 协议分层：NVMe-MI、MCTP、SMBus 与 PCIe

### 2.1 最容易混淆的一点

NVMe-MI、MCTP、SMBus 和 PCIe 不在同一个抽象层级。把它们全称作“协议”没错，但它们分别回答不同的问题：

| 名称 | 核心问题 | 类比 |
|---|---|---|
| NVMe-MI | “我要对 SSD 做什么管理操作？” | 应用层协议 |
| MCTP | “管理消息怎样寻址、分段、送到终点？” | 传输协议 |
| SMBus/I2C | “字节怎样在管理线缆上传？” | 具体道路 |
| PCIe VDM | “管理消息怎样借 PCIe 链路传？” | 另一种道路 |

### 2.2 典型封装关系

当使用 SMBus 时，可按下面的方向理解：

```text
┌────────────────────────────────────────┐
│ NVMe-MI：例如“读取控制器健康状态”           │
├────────────────────────────────────────┤
│ MCTP：EID、消息类型、分段、完整性           │
├────────────────────────────────────────┤
│ MCTP over SMBus binding：SMBus 上如何装帧│
├────────────────────────────────────────┤
│ SMBus / I2C：SCL、SDA、设备地址、交易      │
└────────────────────────────────────────┘
```

若选择 PCIe VDM，则最下两层改为：

```text
NVMe-MI → MCTP → MCTP over PCIe VDM → PCIe Link
```

同一条 NVMe-MI 请求的业务含义不变，变化的是它经过的运输方式。

### 2.3 记住方向，避免倒置

从应用往硬件看是“封装”：NVMe-MI 被装入 MCTP，再装入 SMBus 或 PCIe VDM 报文。从硬件到应用看是“解封装”。

```text
BMC 发出：NVMe-MI 请求
            ↓ 封装
          MCTP 消息
            ↓ 封装
          SMBus 交易
            ↓
           SSD
```

---

## 3. NVMe-MI 与普通 NVMe 的关系

### 3.1 它们是同一家族，不是互相替代

“普通 NVMe”通常是指 NVMe Base 规范定义、通过 PCIe 队列运行的命令接口。它主要处理主机数据访问。NVMe-MI 是面向管理面的补充规范。

```text
                 ┌─ NVMe I/O Commands：读、写、刷新等
NVMe Base ───────┤
                 └─ NVMe Admin Commands：识别、日志、配置等

NVMe-MI ─────────── 管理消息与管理命令；可封装部分 Admin Command
```

### 3.2 NVMe Admin Command 为什么重要

很多 SSD 信息本来就在 NVMe Admin 命令和日志页中，例如：

- Identify Controller：型号、序列号、能力；
- Get Log Page：SMART / Health Information；
- Firmware 操作或控制器管理功能。

NVMe-MI 可以在适用时把某些 NVMe Admin Command 作为“隧道中的载荷”带外送到设备。这不意味着所有 Admin Command 在带外都可用；是否允许取决于规范与设备实现。



---

## 4. Basic Management Command 与 NVMe-MI 的区别

### 4.1 先区分两条“管理门”

有些 NVMe SSD 实现了 SMBus 上的 **Basic Management Command**（常简称 BMC 命令；这里的 BMC 是命令名称，不是 Baseboard Management Controller）。它常用于直接取得基础状态，例如温度。

```text
路径 A：Basic Management Command
BMC → SMBus Block Read / Write → SSD

路径 B：NVMe-MI
BMC → NVMe-MI → MCTP → NVMe-MI → SSD
```

路径 A 更短；路径 B 是更通用、可扩展的管理体系。两者不是“新旧版本的一对一替代”，也不是所有 SSD 都同时实现。

### 4.2 Basic Management Command 的直觉

它类似一个设备在 SMBus 上提供的简易状态窗口：管理控制器以该设备的 SMBus 从地址发起块读/写，并使用约定的命令码取得有限信息。工程实践中经常能见到地址 `0x6a`，但这只是常见约定或平台选择，**不能当成所有 SSD 的通用事实**。

```text
SMBus 地址（例：0x6a）
       │
       ├─ 命令码（例：某个基础管理命令）
       └─ 返回状态/温度等有限字段
```

它无需 MCTP 的 EID、路由或消息类型协商；BMC 知道 SMBus 地址即可直接发起交易。

### 4.3 NVMe-MI 的优势

NVMe-MI + MCTP 提供了更系统化的能力：

- MCTP 层可使用逻辑端点地址（EID）和标准控制命令发现设备；
- 可查询设备支持的消息类型，从而判断是否支持 NVMe-MI；
- 可承载比单次 SMBus 基础读写更丰富的管理事务；
- 可换用不同传输绑定，例如 SMBus 或 PCIe VDM。

### 4.4 实际设计时应问的三个问题

1. SSD 的数据手册明确支持哪一种带外接口？
3. 你需要的只是固定的温度字段，还是完整的标准化管理能力？

不要因为在某个软件组件中看到 `0x6a`，就假设某块 M.2 SSD 一定支持 Basic Management Command；也不要因为 SSD 是 NVMe，就假定它一定暴露了 MCTP/NVMe-MI 端点。

---

## 5. MCTP 基础

MCTP（Management Component Transport Protocol）可以理解为管理消息的“小型网络层”。它不关心“温度的数值是什么”，而关心“消息怎样到达正确的管理端点”。

### 5.1 Endpoint：能收发 MCTP 消息的端点

MCTP Endpoint 是实现 MCTP 协议、可以接收和响应 MCTP 消息的组件。它可能是 SSD、NIC、CXL 设备、电源模块或其他管理设备。

**一颗出现在 I2C 总线上的芯片，不自动等于 MCTP Endpoint。**它必须实际支持相应的 MCTP 传输绑定和控制协议。

### 5.2 EID：MCTP 网络里的逻辑地址

EID（Endpoint ID）是 MCTP 网络中用来寻址端点的标识。可以把它理解为同一个管理网络内的“房间号”。

```text
SMBus 从地址：总线交易要敲哪扇门（链路层地址）
MCTP EID：     管理消息要交给哪个端点（逻辑地址）
```

两者常常同时出现，但绝不是同一个数字，也不应互相替代。

### 5.3 Network ID：网络范围的标识

MCTP 可能包含多个网络。Network ID（网络 ID）用于在路由/软件管理层区分这些 MCTP 网络。它的具体表现会随操作系统和 MCTP 实现而不同。

对初学者最实用的理解是：

```text
Network ID：先确定“在哪个 MCTP 网络”
EID：        再确定“这个网络中的哪个端点”
```

在一个简单、单总线的 BMC 场景里，你可能暂时感觉不到 Network ID 的存在；但它在多个绑定、桥接或路由场景中不可省略。

### 5.4 Message Type：端点内的“业务窗口”

同一个 Endpoint 可处理多种 MCTP 消息类型。Message Type 表示 MCTP 载荷应交给端点内哪个协议处理。

```text
MCTP Endpoint（EID = 0x12）
  ├─ MCTP Control：发现、EID 配置、能力查询
  ├─ NVMe-MI：NVMe 设备管理
  └─ 其他可能支持的管理协议
```

所以“能 ping 到 EID”最多证明它是 MCTP 端点；只有查询其 Message Type Support 后，才能判断它是否能够接收 NVMe-MI。

### 5.5 四个概念放在一张图里

```text
BMC 的 MCTP 软件
     │ 在 Network A 中向 EID 0x12 发消息
     ▼
MCTP network A
     │ 经 SMBus 物理地址 0x2a 投递
     ▼
SSD 管理端点
  EID: 0x12
  支持的 Message Types: Control, NVMe-MI
```

---

## 6. MCTP 如何发现设备

### 6.1 发现不是“扫描 I2C 地址”这么简单

I2C/SMBus 扫描最多说明某个地址有应答；它不能可靠告诉你：

- 应答者是否是 MCTP Endpoint；
- 它的 EID 是什么；
- 是否有稳定的 UUID；
- 是否支持 NVMe-MI。

因此完整发现应分成“物理可达”与“协议可管理”两步。

### 6.2 Bus Owner：谁负责组织发现

在 MCTP over SMBus 场景中，**Bus Owner** 是负责该 SMBus MCTP 网络管理与发现的角色。BMC 往往承担它：它知道总线拓扑、发起 MCTP Control 命令、为未配置端点分配 EID，并维护端点信息。

```text
BMC（Bus Owner）
   │
   ├─ 选择 PCA9544 的一个下游通道
   ├─ 检查该分支上的物理设备
   ├─ 通过 MCTP Control 询问端点身份和能力
   └─ 记录：物理地址 ↔ EID ↔ UUID ↔ 支持的消息类型
```

### 6.3 基本发现流程

以下流程刻意省略具体字节编码，突出顺序和目的：

```text
1. 找到可通信的物理设备
       │
2. Get Endpoint ID
       │  返回当前 EID，以及端点是否已配置
       ▼
3. 必要时 Set Endpoint ID
       │  由 Bus Owner 分配一个网络内唯一的 EID
       ▼
4. Get Endpoint UUID
       │  获得更稳定的设备身份（若设备支持/提供）
       ▼
5. Get Message Type Support
       │  获得它可接收的 MCTP 载荷类型
       ▼
6. 找到 NVMe-MI Message Type
       │
       ▼
7. 以该 EID 发送 NVMe-MI 请求
```

### 6.4 Get Endpoint ID

这是 MCTP Control 类的查询。它回答：这个端点现在使用什么 EID？该 EID 是静态设置、动态设置，还是尚未设置？

当一个端点刚接入管理网络时，可能没有可用 EID，或使用临时/默认状态。Bus Owner 不应只凭猜测就以某个 EID 向它发送业务请求。

### 6.5 Set Endpoint ID

如果端点可由 Bus Owner 配置，且尚未有合适 EID，Bus Owner 会发送 Set Endpoint ID 分配一个在该网络中唯一的 EID。

```text
错误示例：bus60 上的 SSD 与 bus61 上的 SSD 都随意使用 EID 0x08
正确原则：同一 MCTP network 中，每个可寻址 endpoint 的 EID 必须唯一
```

实际产品中 EID 的持久性、重启后是否保留、是否允许修改，都取决于端点能力和平台策略。

### 6.6 UUID：跨时间识别“同一个设备”

EID 像酒店房号，可能重分配；UUID 更像设备的身份证号。Get Endpoint UUID 用于获得一个全局唯一、较稳定的身份标识（前提是端点支持相应能力）。

这有助于处理热插拔或总线重新枚举：即使某一轮发现中 EID 改变，管理软件仍可判断它是否是原来的那块 SSD。

### 6.7 Get Message Type Support：确认 NVMe-MI

这是发现的关键收尾。查询响应中如果表明支持 NVMe-MI 对应的 MCTP Message Type，BMC 才能合理地尝试 NVMe-MI 通信。

```text
“SMBus 地址有响应”       → 仅证明物理层可能可达
“Get Endpoint ID 有响应”  → 证明它是可通信的 MCTP endpoint
“支持 NVMe-MI type”       → 才证明它可接收 NVMe-MI 消息
```

### 6.8 将示例拓扑代入流程

```text
BMC
 │
 ├─ bus4：向 PCA9544 选择通道 0 ──► bus60 ──► SSD #0
 │                                       ├─ physical SMBus address: 0x2a（示例）
 │                                       ├─ EID: 0x12（发现或分配后）
 │                                       └─ types: Control + NVMe-MI
 │
 ├─ 选择通道 1 ───────────────► bus61 ──► SSD #1
 ├─ 选择通道 2 ───────────────► bus62 ──► SSD #2
 └─ 选择通道 3 ───────────────► bus63 ──► SSD #3
```

注意：如果 PCA9544 的多个分支被 MCTP 软件视作同一网络，就必须跨所有分支保证 EID 唯一；如果平台把它们划入不同 MCTP 网络，则 Network ID 也必须参与区分。具体归属由平台拓扑和软件设计决定。

---

## 7. MCTP over SMBus

### 7.1 它是什么

MCTP over SMBus 是一种“绑定”（binding）：它规定 MCTP 报文怎样放进 SMBus 交易中、怎样使用 SMBus 地址、怎样处理分段与校验。MCTP 的 EID 仍在 MCTP 报头中；SMBus 从地址仍服务于总线访问，两者各司其职。

### 7.2 用 PCA9544 场景理解路径

PCA9544 是 I2C MUX（多路复用器）。它不是 MCTP 路由器，也通常不理解 NVMe-MI；它的作用是选择哪一条下游 I2C 支路与上游相连。

```text
BMC I2C controller
       │
     bus4
       │
    [PCA9544]
   ┌───┼───┬───┐
 bus60 bus61 bus62 bus63
   │     │     │     │
 SSD0  SSD1  SSD2  SSD3
```

Linux 把 MUX 通道呈现为 `bus60` 至 `bus63` 之类的逻辑总线，是为了让上层软件能以正常 I2C 设备方式访问每条下游支路。硬件上的电气路径仍是 `bus4 → PCA9544 → 某条通道`。

### 7.3 一次发送的概念过程

```text
1. BMC 选择 PCA9544 的目标通道，例如通向 bus60
2. BMC 对 SSD 的 SMBus 从地址发起交易
3. 交易数据中携带符合 SMBus binding 的 MCTP packet
4. SSD 解析 MCTP：检查目标 EID、消息类型、分段等
5. SSD 将 NVMe-MI 载荷交给其管理逻辑
6. 响应沿相反方向返回
```

### 7.4 PEC 与可靠性

SMBus 可以使用 PEC（Packet Error Code）检测传输错误。PEC 是 SMBus 交易完整性的机制，不等于 MCTP 的消息身份、分段或 NVMe-MI 的命令状态。

可以把它们分开理解：

```text
PEC：       这一次总线传输的字节有没有被传坏？
MCTP：      报文是不是送给正确 endpoint，多个包能否拼成一条消息？
NVMe-MI：   SSD 是否成功执行了该管理命令？
```

### 7.5 为什么 `i2cdetect` 不足以做结论

I2C 探测工具对某个地址有反应，只能作为“线路/地址可能存在设备”的线索。它无法代替规范化的 MCTP 控制发现，并且主动探测本身在某些设备或平台上也可能有副作用。是否支持 MCTP/NVMe-MI，必须通过设备文档和协议级能力查询确认。

---

## 8. MCTP over PCIe VDM

### 8.1 另一条道路，不是另一套 NVMe-MI

PCIe VDM 是 PCIe 中的 Vendor Defined Message 机制。MCTP over PCIe VDM 规定怎样把 MCTP 消息封装进 PCIe VDM。上层仍然可以是相同的 MCTP Control 或 NVMe-MI。

```text
方案 1：BMC → MCTP → SMBus binding → I2C/SMBus 线 → SSD
方案 2：管理方 → MCTP → PCIe VDM binding → PCIe 链路 → SSD
```

### 8.2 与 SMBus 方案的直觉差异

| 维度 | MCTP over SMBus | MCTP over PCIe VDM |
|---|---|---|
| 物理路径 | 管理侧带外引脚/总线 | PCIe 链路 |
| 常见管理方 | BMC | 可访问 PCIe 管理消息的组件 |
| 硬件依赖 | SMBus 接线、MUX、地址 | PCIe 拓扑和 VDM 支持 |
| 上层管理语义 | MCTP / NVMe-MI | MCTP / NVMe-MI |

“使用 PCIe VDM”不自动表示 BMC 能访问它；“SSD 接在 PCIe 上”也不自动表示平台为 BMC 打通了 VDM 管理路径。应以 SSD、主板和 BMC/主机架构的能力为准。

### 8.3 何时更适合考虑它

如果设计希望避免额外 SMBus 管理布线，或平台已有 PCIe MCTP/VDM 管理架构，PCIe VDM 可能合适。相反，如果 BMC 直接连接到 M.2 槽位的 SMBus 管理引脚，SMBus binding 往往更贴近板级现实。

---

## 9. NVMe-MI 如何发现和管理 SSD

### 9.1 有两层“发现”，不要混在一起

```text
第一层：MCTP 发现
        找到“这是一个 MCTP endpoint；EID 是多少；支持什么消息类型”

第二层：NVMe-MI / NVMe 设备识别
        找到“这是什么 SSD；它有何能力；能读取哪些管理信息”
```

MCTP 发现的是**通信对端**；NVMe-MI 发现/识别的是对端背后的**NVMe 设备及其管理能力**。

### 9.2 推荐的心智流程

```text
SMBus 支路可达
       ↓
MCTP Control：拿到 EID、UUID、Message Type Support
       ↓
确认支持 NVMe-MI
       ↓
发送 NVMe-MI：查询管理能力/设备状态
       ↓
必要时使用适用的 NVMe Admin Command 通道识别控制器
       ↓
建立“槽位 ↔ UUID/EID ↔ SSD 序列号/型号”的资产关系
```

### 9.3 为什么槽位、EID、序列号都要记录

它们回答不同问题：

| 标识 | 回答的问题 | 示例 |
|---|---|---|
| 物理槽位/支路 | SSD 插在哪里？ | PCA9544 通道 0，bus60 |
| SMBus 地址 | 当前如何在这条线缆上访问？ | `0x2a`（示例） |
| EID | 当前 MCTP 网络中向谁发消息？ | `0x12`（示例） |
| UUID | 这是哪个 MCTP 端点？ | 稳定的端点身份 |
| NVMe 序列号 | 这是哪块存储设备？ | 厂商序列号 |

热插拔、MUX 重置和重新发现可能改变其中某些映射，因此不能只保留一个数字。

### 9.4 “支持 NVMe-MI”仍不是“所有功能都支持”

设备支持 NVMe-MI Message Type 后，还应根据其能力和返回状态判断具体管理命令是否可用。厂商固件、MI 版本、控制器状态和平台权限都可能影响最终行为。

---

## 10. NVMe-MI 命令模型

### 10.1 请求与响应，而不是 NVMe I/O 队列

普通 NVMe I/O 的核心是主机内存队列、Submission Queue 和 Completion Queue；NVMe-MI 更接近一次管理请求对应一次管理响应的模型。

```text
普通 NVMe：Host 在内存队列投递命令 → SSD DMA/处理 → Completion Queue

NVMe-MI： BMC 发管理请求 → MCTP 传输 → SSD 返回管理响应
```

### 10.2 命令的三层结果

分析一次失败时，应分层看状态：

```text
SMBus 失败：没有完成底层总线交易
MCTP 失败：没有收到匹配 endpoint 的有效协议响应
NVMe-MI 失败：消息到达设备，但管理命令被拒绝或执行失败
```

这三类失败的含义完全不同。不要把“温度读不到”笼统归为“SSD 不支持”。

### 10.3 常见的管理载荷类型

从概念上，NVMe-MI 交互常包括：

- MI 自身定义的管理命令：查询/管理设备相关能力和状态；
- 传递适用的 NVMe Admin Command：在带外路径获得标准 NVMe 控制器信息或日志；
- 响应状态与数据：表明命令成功与否，并携带结果。

具体命令编号、报头字段和允许的 Admin Command 范围应以目标 NVMe-MI 规范版本及 SSD 数据手册为准。实现时不要从本文的概念图直接推导二进制报文。

### 10.4 报文的概念剖面

```text
[SMBus transaction]
    └─ [MCTP header]
          ├─ destination EID
          ├─ source EID
          ├─ message type = NVMe-MI
          └─ [NVMe-MI request/response]
                ├─ operation / command
                ├─ request identifier / tag（实现相关）
                ├─ command data
                └─ completion status + response data
```

真实字段的大小、排列和分段方式由各层规范精确定义；这张图只表达“谁包着谁”。

---

## 11. NVMe-MI 如何获取 SSD 温度

### 11.1 先确定“温度来自哪里”

SSD 温度常见来源不是一个单独的“读温度命令”，而是设备健康/SMART 信息中的温度字段。对 NVMe SSD，标准 NVMe SMART / Health Information Log 中定义了 Composite Temperature，设备也可能提供额外的温度传感器字段。

```text
SSD 内部传感器
      ↓
NVMe 控制器维护健康/日志信息
      ↓
通过适用的 NVMe-MI 管理路径读取
      ↓
BMC 将原始值转换、校验并呈现为温度传感器
```

### 11.2 概念上的带外读取过程

```text
1. BMC 已完成 MCTP 发现
   - 已知目标 SSD 在 bus60 支路
   - 已知 physical address、EID、UUID
   - 已确认支持 NVMe-MI

2. BMC 向 EID 发送 NVMe-MI 请求
   - 请求相应的健康信息，或传递被允许的 NVMe Admin Get Log Page

3. SSD 返回健康日志/管理响应
   - 包含 Composite Temperature 等字段（具体以设备能力为准）

4. BMC 解析并输出
   - 处理单位、无效值、超时与状态码
   - 将该值关联到物理槽位，例如 M.2 SSD #0
```

### 11.3 温度单位：常见的开尔文陷阱

NVMe 健康日志的 Composite Temperature 通常以开尔文（K）表达。面向用户展示摄氏度时，应明确转换关系：

```text
摄氏度 = 开尔文 − 273.15

示例：303 K ≈ 29.85 °C
```

具体字段的有效范围、保留值和传感器含义应严格按所使用 NVMe 规范版本解释。不能因为字段数值看起来像温度，就不做有效性检查。

### 11.4 Composite Temperature 与多个传感器

- **Composite Temperature**：控制器定义的综合温度，常适合作为通用的“SSD 温度”。
- **额外温度传感器**：部分设备报告更多温度点，例如控制器、NAND 或板上其他位置；是否有、含义为何，都依赖设备实现。

对 BMC 管理界面而言，合理策略往往是先稳定展示 Composite Temperature，再在设备明确提供且语义清晰时增加扩展传感器。

### 11.5 温度不可读时，按层定位概念问题

虽然本文不展开调试步骤，但可以用这棵思维树判断问题属于哪一层：

```text
读不到温度
 ├─ 没有物理连通性？
 │   └─ SMBus 支路、MUX 通道、供电、地址/接线问题
 ├─ 不是可用的 MCTP endpoint？
 │   └─ Get Endpoint ID 或 MCTP binding 不成立
 ├─ endpoint 不支持 NVMe-MI？
 │   └─ Message Type Support 不包含 NVMe-MI
 ├─ 支持 MI，但该命令不可用？
 │   └─ SSD 能力、规范限制、控制器状态或厂商实现差异
 └─ 收到数据但解释错误？
     └─ 单位、字段偏移、保留值或状态码处理错误
```

### 11.6 最终串起来

```text
BMC
 │
 │ bus4
 ▼
PCA9544 ── 选择通道 0 ──► bus60 ──► M.2 SSD #0
                                         │
                                         │ MCTP Control discovery
                                         ▼
                                  EID 0x12，支持 NVMe-MI
                                         │
                                         │ NVMe-MI health request
                                         ▼
                                 SMART/Health temperature data
                                         │
                                         ▼
                               BMC 呈现“SSD #0：30 °C”
```

---

## 结语：用三句话掌握主线

1. **NVMe-MI 是 SSD 的带外管理语言**；它让 BMC 不依赖主机 OS 也能管理支持该接口的 NVMe 设备。  
2. **MCTP 是消息运输和寻址层**；先发现 Endpoint、EID 和 Message Type，再谈 NVMe-MI。  
3. **SMBus 或 PCIe VDM 是承载道路**；在本文的 `BMC → bus4 → PCA9544 → bus60~63 → M.2 SSD` 场景中，PCA9544 只负责选择物理支路，MCTP/NVMe-MI 的协议语义仍在两端完成。

当你下次看到“bus60、地址 0x2a、EID 0x12、NVMe-MI”这几个值时，可以这样读：`bus60` 表示 Linux 可访问的 I2C 分支，`0x2a` 是该分支上的 SMBus 物理地址，`0x12` 是 MCTP 逻辑端点地址，而 NVMe-MI 才是要让 SSD 执行的管理协议。
