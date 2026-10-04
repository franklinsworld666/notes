# OpenBMC Host USB Network 原理与架构说明

> 本文以常见的“BMC 作为 USB Device，Host 作为 USB Host”架构为例。具体控制器、Gadget 创建方式、网络管理组件与接口名称应以目标平台为准。

## 一、概述：Host USB Network 是什么

Host USB Network 是 BMC 与主机操作系统之间的一条以 USB 为传输介质的网络链路。BMC 在 USB 总线上呈现一个以太网设备，Host 将其识别成网络接口；两侧再通过各自的 Linux 网络栈交换以太网帧和 IP 数据包。上层程序通常按普通网卡使用它，可承载 Host Agent 通信、SSH、HTTPS、Redfish 等。

这条链路通常位于服务器内部，和 Host 接入业务网络、BMC 接入外部管理网络是不同的接口。它是否与其他网络互通，取决于额外的桥接、路由和防火墙配置；建立 USB Network 本身不会自动开放外部网络。

```text
BMC（USB Device）                                Host（USB Host）┌───────────────────────────────┐     USB     ┌──────────────────────────────┐
│ USB Device Controller / UDC   │◀──────────▶│ USB Host Controller          
│             ↑                 │             │             ↓                
│ Linux USB Gadget              │             │ USB 枚举与设备驱动           
│             ↑                 │             │             ↓                
│ ECM / NCM / RNDIS Function    │             │ USB Ethernet 网卡驱动       │
│             ↑                 │             │             ↓                
│ usb0 / usbX + BMC IP          │◀── IP 网络 ─▶│ enx... / 其他名称 + Host IP 
└───────────────────────────────┘             └──────────────────────────────┘
```

这里的“开启成功”有多个阶段：BMC 出现 `usb0`，通常表示网络 Function 已创建；Gadget 绑定 UDC 后才向物理 USB 链路提供设备；Host 枚举后才有自己的网卡；两侧有合适的 IP 与路由后才能进行 IP 通信；SSH、Redfish 还要求相应服务可达。**不能仅凭 BMC 有 `usb0` 判断整条链路已通。**

## 二、分层模型总览

| 层次 | 主要对象 | 该层成功的可观察证据 |
| --- | --- | --- |
| 1. 物理与硬件 | 板内 USB 连接、Host Controller、BMC Device Controller | Host 能检测到稳定的 USB 设备连接 |
| 2. BMC 控制器 | Device 模式、UDC 驱动 | BMC 可列出可用 UDC |
| 3. BMC Gadget | Gadget、Configuration、Function、UDC 绑定 | Gadget 配置存在且已绑定 UDC |
| 4. USB 以太网协议 | ECM、NCM 或 RNDIS | Host 识别到相应 USB 网络功能 |
| 5. Host 系统 | USB 枚举、USB 网卡驱动、Linux 网络接口 | Host 出现对应网络接口 |
| 6. IP 网络 | 两端地址、接口状态、路由 | 可互相访问对端 IP |
| 7. 网络管理 | NetworkManager、systemd-networkd 等 | 配置在重连和重启后可重新应用 |
| 8. 应用服务 | Host Agent、SSH、Redfish | 目标服务可以按预期访问 |

定位问题时，从最早失败的层开始检查。例如 Host 连 USB 设备都没有检测到，先检查物理连接、控制器角色和 Gadget 绑定；若 Host 网卡稳定存在而 IP 配置变化，重点转到网络管理层。

## 三、Layer 1：物理层与硬件拓扑

典型服务器中，BMC 与 Host 的 USB 连接由主板内部走线或内部集线器实现。BMC 侧的 USB 控制器工作在 **Device/Peripheral** 模式，Host 侧的控制器工作在 **Host** 模式。Host 发起枚举，读取 BMC 提供的描述符，再为其功能选择驱动。BMC 则响应控制请求并收发后续数据。

USB Host 与 USB Device 是协议角色，不等于网络通信的主动端与被动端。完成枚举并建立网络后，Host 和 BMC 上的应用都可以主动发起 TCP/IP 连接。

此层应先确认目标硬件确实把 BMC 的 Device 控制器连接到了 Host 可见的 USB 端口，以及相关电源、复位、复用和固件配置允许该路径工作。不同主板可能在 BMC 或 Host 重启时切断该路径，因此不能把其他平台的时序当作通用保证。

**典型异常：**Host 完全没有设备连接事件；设备反复断开、重连；BMC 复位时 Host 端网卡消失。这些现象还不能单独区分硬件与 Gadget 问题，需要与 BMC 日志和 UDC 状态对照。

## 四、Layer 2：BMC USB 控制器与 UDC

UDC（USB Device Controller）是 Linux Gadget 框架对 USB Device 控制器的抽象。UDC 驱动负责直接操作控制器硬件，包括端点、控制传输、中断和数据收发；Gadget 框架通过统一接口使用这些能力。

```text
Gadget / Ethernet Function
            ↓
Linux Gadget 核心与 UDC 接口
            ↓
平台 UDC 驱动
            ↓
BMC USB Device Controller
            ↓
Host 可见的 USB 链路
```

在 BMC 上查看 UDC：

```bash
ls -l /sys/class/udc/
```

若目录为空，应检查控制器驱动、设备树、角色配置以及平台服务。**能列出 UDC 只说明具备可绑定的控制器，并不代表 Gadget 已创建或 Host 已完成枚举。**

## 五、Layer 3：BMC Linux USB Gadget 与 ConfigFS

### 5.1 Gadget 的组成

Linux USB Gadget 让运行 Linux 的设备充当 USB 外设。一个 Gadget 可包含设备描述符、一个或多个 Configuration，以及每个 Configuration 下的 Function。Function 是具体对外提供的能力，例如 USB 串口、存储或以太网。Host 在枚举过程中读取描述符，选择配置并绑定相应驱动。

```text
USB Gadget
├── 设备描述符：VID、PID、厂商/产品/序列号等
├── Configuration：供 Host 选择的功能组合
│   └── Ethernet Function：ECM、NCM 或 RNDIS
└── UDC 绑定：把上述配置交给实际 USB 控制器
```

### 5.2 ConfigFS 如何配置 Gadget

ConfigFS 提供运行时配置入口，常见位置为 `/sys/kernel/config/usb_gadget/`。基于 ConfigFS 的一般顺序是：创建 Gadget 和描述符；创建 Configuration；创建 Ethernet Function；把 Function 链接到 Configuration；最后向 Gadget 的 `UDC` 属性写入 UDC 名称以启用它。平台也可能使用预编译 Gadget 驱动或由 systemd 服务执行这些步骤，因此不能假设所有 OpenBMC 镜像都以同样目录结构部署。

只读检查示例：

```bash
ls /sys/kernel/config/usb_gadget/
find /sys/kernel/config/usb_gadget -maxdepth 3 -type d
cat /sys/kernel/config/usb_gadget/<gadget-name>/UDC
```

`<gadget-name>` 需要替换为实际名称。`UDC` 文件中若有控制器名称，表示该 ConfigFS Gadget 当前已绑定；空值则表示未绑定。绑定是让 Host 能枚举该 Gadget 的关键步骤。

某些 Ethernet Function 在创建时就会生成 BMC 侧网络接口。因此可能出现 **BMC 已有 `usb0`，但 Gadget 尚未绑定 UDC，Host 完全看不到网卡** 的状态。具体接口创建时机由 Function 和平台实现决定，应以现场观察为准。

### 5.3 重建与重新枚举

如果创建 Gadget 的服务重启、Gadget 解绑/重新绑定 UDC，或 BMC 重启，Host 可能收到 USB 断开事件并重新枚举。Host 上旧网络接口被删除时，属于该接口的运行态 IP、路由和统计信息也会消失；新接口即使命名相同，也是一次新的设备生命周期。

## 六、Layer 4：USB Ethernet Function

Ethernet Function 把 Linux 网络栈中的以太网帧封装到 USB 传输中，并在对端驱动还原。它决定 Host 需要识别哪一类 USB 网络功能。

| Function | 主要特点 | 选择时关注 |
| --- | --- | --- |
| ECM（CDC Ethernet Control Model） | 常见的 USB CDC 以太网功能 | Linux Host 驱动支持、平台兼容性 |
| NCM（CDC Network Control Model） | 支持将数据组织成较大的传输块，适合更高吞吐需求 | 双端 NCM 支持、性能与兼容性验证 |
| RNDIS | 面向特定 Host 兼容需求的 USB 网络协议 | Host 驱动和产品安全/维护策略 |

协议的选用由 BMC Gadget 配置和 Host 支持情况共同决定。Linux Host 上常见对应驱动包括 `cdc_ether`、`cdc_ncm`、`rndis_host`，但实际绑定驱动要看设备描述符和当前内核配置，可在 Host 上用 `lsusb -t`、`ethtool -i <ifname>` 或内核日志确认。

每条 USB 以太网链路有两端 MAC：`dev_addr` 对应 Gadget/BMC 一端，`host_addr` 对应 Host 一端。ConfigFS Ethernet Function 可能在未指定时生成随机 MAC；如果每次启动发生变化，Host 的接口命名或 NetworkManager 的连接匹配可能跟着变化。量产设计应明确 MAC 的生成与持久化策略，并避免两端或多台设备 MAC 冲突。函数目录中的 `ifname` 可帮助确认 BMC 实际使用的接口名。

## 七、Layer 5：BMC 侧网络接口与网络栈

Ethernet Function 在 BMC Linux 网络栈中通常对应 `usb0`，但也可能是 `usb1` 或其他名称。它是一个普通的 Linux 网络接口：有 MAC、MTU、收发计数、接口状态和可配置的 IP。区别在于它的底层传输走 USB Gadget，而不是外部物理以太网口。

```bash
ip -br link
ip -br address
ip route
```

以静态 IPv4 为例：

```text
BMC：usb0       192.168.100.1/24
Host：USB NIC    192.168.100.2/24
```

对同一网段内的直接通信，两端有互不冲突且掩码匹配的地址即可，通常无需为这条链路设置默认网关。接口 `UP`、地址、路由、邻居发现和防火墙都可能影响通信。`ip link` 能看接口，`ip addr` 才能看 IP；看见接口不表示已经有 IPv4 地址。

OpenBMC 平台的持久化网络配置方式不完全一致。常见组合是 `systemd-networkd` 负责向内核应用网络设置，`phosphor-networkd` 等组件提供 OpenBMC 管理接口并协调配置；有些平台还使用定制服务或镜像默认文件。应检查当前镜像实际启用的服务和该 USB 接口的配置来源，不要从项目名称推断所有平台行为。

## 八、Layer 6：Host USB 枚举与网卡驱动

当 Gadget 与 UDC 绑定并且链路可用时，Host 检测到 USB Device，读取设备与接口描述符，选择 Configuration，为 Ethernet Function 绑定 USB 网络驱动，最后由 Linux 网络栈创建网络接口。

```text
BMC 绑定 UDC → Host 检测 USB 设备 → 读取描述符
             → 绑定 ECM/NCM/RNDIS 驱动 → 创建 Host USB 网卡
```

Host 端常用检查：

```bash
sudo dmesg --follow
lsusb -t
ip -br link
ip -br address
```

接口可能叫 `enx<MAC>`、`usb0`，也可能使用包含 USB 拓扑信息的可预测名称；名称由系统的命名策略决定。**应通过 USB 设备、驱动、MAC 和系统日志确认目标接口，不能只按 `usb0` 猜测。**

当 Gadget 停止或重建时，Host 可能删除旧接口，再创建新接口。新的名称可能相同，也可能变化。若只看到 IP 消失，先确认网络接口是否也发生了删除/创建：接口对象消失属于 USB/设备生命周期问题；接口保持存在而地址被移除，则继续检查地址管理策略。

## 九、Layer 7：两端 IP 与网络管理

### 9.1 IP 层何时可用

要通过 IPv4 从 Host 访问 BMC，两端都需要可用 IPv4 地址，以及可达的路由。最简单的方式是处于同一子网，例如上文的 `192.168.100.1/24` 和 `192.168.100.2/24`。地址可以来自静态配置或 DHCP；IPv6 Link-Local 也可在具备相应支持时用于本链路，并需要在使用时指定接口作用域。

USB 网卡枚举成功与 IP 配置完成是两个不同阶段。`ip addr add` 直接修改当前内核中的接口地址，适合临时验证；它不创建 NetworkManager 连接配置，也不修改 `systemd-networkd` 的 `.network` 文件。接口重新创建或网络管理服务重新应用配置后，手工地址可能消失。

还需注意：**接口处于 `DOWN` 通常不会使已经成功添加的地址在 `ip addr` 输出中隐藏。**若命令成功后立即看不到地址，应先核对命令退出状态、接口名称和接口是否被重建，再查是否有管理服务重配。

### 9.2 当前已解决案例：Ubuntu Host 使用 NetworkManager

本案例中，Host 使用 Ubuntu，USB 网卡由 NetworkManager 管理。现场最终通过 Ubuntu 设置界面为该连接配置 IP，问题已解决。这说明该 Host 的最终地址配置应交给 NetworkManager 的连接配置保存与重连时恢复。它**不意味着 NetworkManager 环境下 `ip addr add` 永远不能成功**；它只是运行态修改，是否被保留取决于连接激活、重应用、DHCP 和接口重建等事件。

图形界面的典型操作是：找到对应的有线/USB 网络连接 → IPv4 → 选择手动 → 填写地址与前缀 → 保存并重新连接。命令行也可通过 `nmcli` 编辑相同的连接配置，避免把地址配置写到错误网卡。检查时可用：

```bash
nmcli device status
nmcli connection show
nmcli device show <host-usb-ifname>
```

若要采用静态配置，应先确认连接名称和接口，再在相应连接的 `ipv4.method`、`ipv4.addresses` 等属性中设置并激活。USB 内部点对点链路通常不需要为这条连接指定默认网关，以免改变 Host 的默认出网路径。

### 9.3 其他网络管理方式

若 Host 或 BMC 使用 `systemd-networkd`，通常由 `.network` 文件按接口名、MAC 等条件匹配网卡并应用地址、DHCP 或路由。若使用 DHCP，应明确哪一端提供服务器、哪一端作为客户端。IPv4 Link-Local 虽可自动配置，但地址发现与稳定性不一定满足固定 Host Agent 的需求。

为同一接口确定一个清晰的管理者。多个管理组件同时修改同一接口，或手工修改与管理服务的期望配置不一致，都会使观察结果变得难以解释。

## 十、Layer 8：上层服务与访问控制

IP 可达后，应用才能尝试使用 Host Agent、SSH、HTTPS/Redfish 等。`ping` 成功只证明一定程度的 IP 连通，不证明 TCP 服务在运行或允许从 USB 接口访问。SSH/Redfish 还取决于服务状态、监听地址、认证和防火墙规则。

```text
Host Agent / ssh / curl
         ↓
TCP 或 UDP、IP
         ↓
Host USB NIC ↔ BMC usb0
         ↓
USB Ethernet 驱动 ↔ Gadget Function
         ↓
板内 USB 链路
```

Host USB Network 常直接连接到 BMC 管理平面。生产环境应确认哪些服务需要暴露在这条链路上，并按产品策略设置认证和访问控制。

## 十一、完整时序：从上电到可通信

1. 主板提供 BMC 与 Host 之间的 USB 物理连接，双方控制器处于预期角色。
2. BMC UDC 驱动就绪，Gadget 配置创建 Ethernet Function。
3. Gadget 绑定 UDC，BMC 开始向 Host 暴露 USB 设备。
4. Host 检测设备、读取描述符、选择配置并绑定 USB 网络驱动。
5. BMC 和 Host 分别出现自己的 Linux 网络接口。
6. 两端网络管理组件为接口设置地址与必要路由。
7. 双方能够交换 IP 数据包；上层服务在允许的范围内接受连接。

这个顺序说明了为什么“BMC 有 `usb0`”“Host 有网卡”“Host 能访问 Redfish”是三种不同的完成状态。实际启动时各服务可能并行，日志时间线比单次截图更能说明先后关系。

## 十二、按层验证与观察

以下命令用于**观察现状**。占位符需替换为实际 Gadget 名称和接口名；不同精简版 BMC 镜像可能不含所有工具。

### BMC：控制器、Gadget、网络接口

```bash
ls /sys/class/udc/
ls /sys/kernel/config/usb_gadget/
cat /sys/kernel/config/usb_gadget/<gadget-name>/UDC
ip -br link
ip -br address
ip route
```

若 Host 尚未枚举，先确认 UDC、Gadget 与物理连接。若 BMC `usb0` 地址不符合预期，检查实际网络管理配置来源。

### Host：USB 枚举、驱动、IP

```bash
lsusb -t
ip -br link
ip -br address
ip route
nmcli device status                 # 使用 NetworkManager 时
networkctl status <host-usb-ifname> # 使用 systemd-networkd 时
```

为了区分接口重建与地址重配，可在不同终端同时观察：

```bash
ip monitor link
ip monitor address
sudo dmesg --follow
journalctl -u NetworkManager -f    # Ubuntu/NetworkManager 场景
```

`ip monitor` 记录**发生了什么变化**，并不能直接证明“谁删除了 IP”。若同时看到 USB disconnect 和接口删除，追查 USB/Gadget 生命周期；若接口持续存在、只有地址变化，再结合 NetworkManager、networkd 或 DHCP 日志寻找配置动作。

### 通信验证

```bash
ping <BMC-USB-IP>
ssh <user>@<BMC-USB-IP>
curl https://<BMC-USB-IP>/redfish/v1/
```

SSH 用户、Redfish 证书与访问权限应使用产品实际配置；仅在明确接受测试环境证书风险时才使用 `curl -k`。若 `ping` 成功而服务失败，继续检查监听端口、认证和防火墙，而不必重新从 USB 物理层开始。

## 十三、结论与已解决案例

OpenBMC Host USB Network 的核心链路是：**物理 USB → BMC UDC → Gadget/以太网 Function → Host USB 枚举与驱动 → 双侧网络接口 → IP 管理 → 应用服务**。每层都有独立的完成条件和观察证据。理解这些边界，才能准确解释“BMC 有 `usb0` 但 Host 没网卡”或“Host 有网卡但地址不稳定”等现象。

本次 Ubuntu Host 案例已通过 NetworkManager 的设置界面完成 IP 配置。这个结果把问题定位在 Host 网络配置管理层：USB 网卡已被识别，但临时 `ip addr add` 并不是该环境中稳定维护地址的配置入口。后续开发应把 USB 功能的建立与 IP 地址的持久化分别设计、分别验证。

## 参考资料

- [Linux 内核：通过 ConfigFS 配置 USB Gadget](https://docs.kernel.org/usb/gadget_configfs.html)：Gadget、Configuration、Function 与 UDC 绑定流程。
- [Linux 内核：Gadget Function 测试说明](https://docs.kernel.org/usb/gadget-testing.html)：ECM/NCM/RNDIS 等 Function 的属性与测试信息。
- [Linux 内核：USB Gadget API](https://docs.kernel.org/driver-api/usb/gadget.html)：Gadget、UDC 和底层控制器驱动的关系。
- [OpenBMC：系统接口架构概览](https://github.com/openbmc/docs/blob/master/architecture/interface-overview.md)：BMC 与 Host 的物理接口及管理网络背景。
- [NetworkManager：`nmcli` 配置示例](https://www.networkmanager.dev/docs/api/latest/nmcli-examples.html)：连接配置与静态 IPv4 的管理方式。
- [NetworkManager：`nm-settings-nmcli` 属性说明](https://www.networkmanager.dev/docs/api/latest/nm-settings-nmcli.html)：`ipv4.method`、`ipv4.addresses` 等连接属性。
