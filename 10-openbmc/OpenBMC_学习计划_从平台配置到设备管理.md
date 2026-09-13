# OpenBMC 学习计划：从平台配置到设备管理

> 适用对象：已有 C++11 实践经验、掌握 Python 基础，并已完成 FRU multirecord、D-Bus 到 Redfish/SNMP、I2C/CPLD 和风扇配置工作的 OpenBMC 工程师。
>
> 建议周期：16 周；每周约 8–10 小时。若工作中有真实平台需求，优先将当周内容落实到一个真实设备上，而不是额外做练习。

## 1. 当前位置与目标

### 当前能力判断

你已经不是“刚接触 OpenBMC”的阶段，而处于**中级工程师的中后段**：可以沿着“硬件 / D-Bus / 对外接口”完成具体功能，并已实际改动过 FRU、bmcweb/SNMP 和平台 I2C 配置。

| 已体现的能力    | 说明                                                                                |
| --------- | --------------------------------------------------------------------------------- |
| FRU 与资产信息 | 已能扩展 multirecord（UUID、MAC），这是偏复杂的平台定制工作                                           |
| D-Bus 消费端 | 已能查询 D-Bus 并经 Redfish、SNMP 对外提供数据                                                 |
| I2C 平台调试  | 已能读 CPLD、定位风扇 bus/address，并通过配置让系统发现设备                                            |
| 当前主要缺口    | “配置 → 实体 / Inventory → Sensor → Health → Redfish/IPMI”的完整生产者链路，以及设备模型与服务生命周期的设计能力 |

### 16 周后应达到的结果

能够独立为一个电源、风扇、硬盘、RAID 卡、GPU 或板载传感器选择合适的数据来源和模块，完成配置、发现、状态/传感器建模、对外暴露与调试闭环；同时能解释每一层为什么这样设计。

```text
硬件（I2C / GPIO / SMBIOS / PCIe / MCTP / PLDM）
        ↓
Linux 驱动、sysfs、或管理协议
        ↓
Entity Manager 配置 / dbus-sensors / 专用服务
        ↓
D-Bus：Inventory、Sensor、OperationalStatus、Association
        ↓
Redfish / IPMI / SNMP / Web UI
```

## 2. 学习原则

1. **先追已有功能，再新增功能。** 每次新增配置前，先在当前平台找一个同类、已工作的设备作为“黄金样例”。
2. **配置不是源码的替代品。** JSON `Exposes` 配置决定“创建什么”，源码决定“谁消费配置、如何读硬件、何时发布 D-Bus”。二者必须一起看。
3. **从 D-Bus 反向验证。** 不先以 Redfish 页面是否显示作为成功标准；依次确认 D-Bus object、interface、property、association、service，再确认 Redfish/IPMI/SNMP。
4. **每种设备产出一页设计卡。** 记录数据源、存在检测、状态定义、D-Bus 路径和接口、关联、Redfish/IPMI 映射、异常注入方法及日志位置。
5. **以你正在维护的分支为准。** OpenBMC 项目与配置 schema 会演进；阅读官方 `master` 时，务必回到产品分支核对实际字段、recipe 和服务名。

## 3. 16 周路线图

| 阶段 | 周数 | 主线 | 可验收产出 |
|---|---:|---|---|
| A. 架构与调试基线 | 1–2 | D-Bus、systemd、构建与观测 | 一张本平台的服务/数据流图；一个可复用的排障记录 |
| B. 配置与资产模型 | 3–5 | Entity Manager、Inventory、FRU、Association | 为一个 PSU 或 GPU 写出完整设计卡与最小配置变更 |
| C. 传感器与健康状态 | 6–8 | dbus-sensors、hwmon、阈值、Presence/Functional | 新增或修改一个真实 Sensor，并完成异常验证 |
| D. 各设备专题 | 9–12 | PSU、磁盘/RAID、GPU、风扇/热管理 | 每类设备至少完成一次源码走读和一项平台改动/验证 |
| E. 协议与质量闭环 | 13–14 | Redfish、IPMI、SNMP、测试 | 一条端到端接口的自动验证脚本或测试用例 |
| F. 独立交付 | 15–16 | 设计、实现、调试、复盘 | 交付一个设备接入功能和简短设计说明 |

## 4. 分周执行计划

### 第 1 周：把已有成果画成系统图

- 选一个你已经完成的功能（建议 FRU ），从启动服务开始追到 Redfish/SNMP 响应。
- 在目标机上记录：相关 systemd service、D-Bus service/object/interface/property、日志关键字、对外 API。
- 阅读：OpenBMC [接口概览](https://github.com/openbmc/docs/blob/master/architecture/interface-overview.md)、[Object Mapper 架构](https://github.com/openbmc/docs/blob/master/architecture/object-mapper.md)、`phosphor-dbus-interfaces` 中你用到的 YAML。
- C++：复习 RAII、`std::variant`、智能指针、lambda、`std::optional`、异常边界；用一个小练习把 D-Bus variant 安全地转换为结构化输出。
- Python：复习 `argparse`、`subprocess`、JSON、异常处理；写一个只读诊断脚本，输出指定 D-Bus object 的接口与属性快照。

**验收：** 能在 10 分钟内说明“某个 Redfish 属性从哪个 D-Bus property 来、哪个服务维护、硬件值如何进来”。

### 第 2 周：systemd、日志和可重复调试

- 阅读当前产品中相关 recipe、service 文件及 `obmc-phosphor-systemd.bbclass` 的使用方式；理解安装、启用、依赖、重启策略。
- 练习使用 `systemctl`、`journalctl`、`busctl`、`dbus-monitor`（或系统现有替代工具）定位一次“配置已加但对象未出现”。
- 阅读官方 [开发速查](https://github.com/openbmc/docs/blob/master/cheatsheet.md) 和 [Yocto 开发文档](https://github.com/openbmc/docs/blob/master/yocto-development.md)。
- 建立一份“新增设备排障顺序”：内核设备 → sysfs/协议读数 → 服务日志 → D-Bus → Association → Redfish/IPMI/SNMP。

**验收：** 对任一失败的配置，能区分是 recipe/服务未启动、Probe 不匹配、`Exposes` 字段不合法、底层设备不可读，还是消费者没做映射。

### 第 3–4 周：Entity Manager、FRU 与 Inventory

- 精读 `entity-manager`：`README.md`、`docs/entity_manager_dbus_api.md`、`docs/my_first_sensors.md`、当前分支的 schema 和你平台现有 JSON。
- 以风扇配置为样例，追踪 `Probe`、`Name`、`Type`、`Exposes` 从 JSON 到 D-Bus configuration object，再到消费者 daemon 的完整路径。
- 阅读 `phosphor-inventory-manager`、`phosphor-fru-device` 和相关 `phosphor-dbus-interfaces/Inventory` YAML；明确 FRU 原始数据、Inventory object、Asset 属性三者的边界。
- 实践：为一个**可识别的电源或 GPU**先只建立 Inventory/Asset/Presence 设计，不急于把所有传感器一次配齐。

**验收：** 可以回答“新增字段应放在 FRU、Entity Manager configuration、Inventory Decorator.Asset，还是 bmcweb OEM 属性”，并给出理由。

### 第 5 周：Association 与设备模型设计

- 精读 D-Bus `xyz.openbmc_project.Association.Definitions` 的 YAML 和当前平台实际对象。
- 为一个设备画出三类关系：`Chassis ↔ Sensors`、`Inventory item ↔ Sensors`、需要时的 `ContainedBy/Contains`。
- 阅读 [传感器架构文档](https://github.com/openbmc/docs/blob/master/architecture/sensor-architecture.md)中关于 Inventory 和 Sensor association 的部分。
- 实践：用 `busctl` 验证新增设备的 object path 命名、inventory 关联、sensor 关联是否与同类设备一致。

**验收：** 新设备的 Redfish 资源若缺失或 sensor 未归属，你能通过 association 找出具体断点。

### 第 6–7 周：Sensor 主线——从硬件值到健康状态

- 精读 `dbus-sensors`：`README.md`、对应 sensor daemon（优先 I2C/hwmon 路径）及你的 `Exposes` 类型；同时看 `phosphor-hwmon` 的配置与 sysfs 映射方式。
- 学习 Linux hwmon：识别 `temp*_input`、`in*_input`、`curr*_input`、`fan*_input`、告警属性及单位换算；确认实际驱动输出的路径和单位。
- 精读 `Sensor.Value`、`Sensor.Threshold.Warning`、`Sensor.Threshold.Critical`、`State.Decorator.Availability`、`State.Decorator.OperationalStatus` YAML。
- 实践：选择一个温度/电压/风扇转速传感器，增加或修改配置；验证数值、单位、阈值、`Available` 与 `Functional`。

**验收：** 能区分以下概念：

| 概念 | 推荐表达 |
|---|---|
| Presence | 物理设备是否插在位 / 是否被探测到 |
| Available | 当前读数/服务是否可用 |
| Functional | 设备是否被判定为工作正常 |
| Threshold alarm | 数值越过 Warning/Critical 阈值的告警状态 |
| Inventory | 设备身份、型号、序列号及状态承载对象 |

### 第 8 周：传感器异常和事件闭环

- 故障注入：断开/模拟不可读的传感器、制造超阈值值、停止相关服务，观察 `Available`、`Functional`、告警、journal 与 SEL/Event Log 的变化。
- 走读 `phosphor-logging`、`phosphor-sel-logger` 的相关调用路径；了解什么情况下应生成事件，避免对瞬时读数抖动直接记录大量 SEL。
- Python：将第 1 周的快照脚本扩展为“基线/当前值”对比工具，输出消失对象、属性变化和异常状态。

**验收：** 能为一个传感器写出“正常、缺失、读失败、越阈值”的预期 D-Bus 与对外接口行为表。

### 第 9 周：电源（PSU）专题

- 对照当前平台已有 PSU，逐项梳理：Presence 信号（GPIO/PMBus/其他）、FRU、PMBus/hwmon 传感器、输入/输出状态、故障位、风扇与电源关联。
- 源码优先级：`entity-manager` 配置 → 对应 `dbus-sensors` daemon / `phosphor-hwmon` → `phosphor-power` 或平台电源服务 → `bmcweb` 中 `PowerSupply` / `PowerEquipment` 处理。
- 阅读 Redfish 的 [bmcweb 支持范围](https://github.com/openbmc/bmcweb/blob/master/docs/Redfish.md)，并以当前产品实现为最终依据。
- 实践：完成一份 PSU 设计卡；优先做 Presence + Inventory + 1–2 个关键 sensor（输入电压、输出功率或温度），再扩展 fault/status。

**验收：** PSU 拔插后，Inventory Presence、相关 Sensor Availability、Redfish PowerSupply 状态和日志行为符合设计。

### 第 10 周：硬盘与 RAID 卡专题

- 先确认硬件管理通道：SATA/SAS 的管理通常不应当被误当作 I2C 设备；可能来自 Host SMBIOS、PCIe、MCTP/PLDM、RAID 控制器管理协议或厂商服务。
- 阅读 `smbios-mdr` 的数据模型及当前平台如何填充 Storage/Drive；梳理“Host 报告的资产信息”与“BMC 直接读到的传感器”的可信边界和更新时机。
- 阅读 `bmcweb` 的 `Storage`、`Drive`、`PCIeDevice` 等相关实现（先从路由/JSON 输出反查 D-Bus 查询）。
- 实践：为一个 RAID 卡或硬盘位写出方案比较：数据源、热插拔 Presence、健康/RAID 状态、容量/序列号、故障事件、Redfish 目标资源。

**验收：** 能明确说明“不在位”“链路不存在”“RAID 降级”“硬盘 SMART 告警”分别由谁检测、在哪一层建模，不把它们混为一个布尔值。

### 第 11 周：GPU 专题

- 先分类 GPU 接入：仅资产展示、PCIe presence、带外温度/功耗传感器、或支持 MCTP/PLDM 的可管理设备；不同类型的实现路径不同。
- 走读当前平台是否已有 PCIe inventory 或 GPU vendor 服务；学习 `SMBIOS`、`PCIeDevice`、`Sensor`、`Association` 的复用边界。
- 制作 GPU 最小接入方案：Presence/Inventory → 关键 sensor → `OperationalStatus` → Redfish 映射；若硬件协议或驱动缺失，记录为平台依赖，不用配置“伪造”状态。
- C++：练习异步错误处理、`sdbusplus::asio` 回调链/协程风格（以当前仓库代码风格为准），重点处理 service restart 和 D-Bus object 消失。

**验收：** 能提交一份 GPU 接入设计，写清哪些数据可靠可交付、哪些需要固件/驱动/厂商协议支持。

### 第 12 周：风扇与热管理深化

- 在已完成风扇发现的基础上，补齐：Presence、双转子 tach、PWM、低速/停转判定、冗余策略、关联的温度输入。
- 走读 `phosphor-fan-presence`、`phosphor-fan-monitor`、`phosphor-fan-control`；有条件再阅读 `phosphor-pid-control`，理解反馈闭环而不是急于修改 PID 参数。
- 实践：为一个风扇故障建立可观测链路：模拟异常 → D-Bus status → event/SEL → Redfish 状态 → 恢复后行为。

**验收：** 能解释“风扇在位但 tach 为 0”和“风扇不在位”在数据模型、告警和热策略上应当不同。

### 第 13 周：Redfish、IPMI、SNMP 的映射选择

- Redfish：从 `bmcweb` 的某一资源 handler 反向追其 D-Bus query；阅读 [bmcweb 开发指南](https://github.com/openbmc/bmcweb/blob/master/DEVELOPING.md)。
- IPMI：走读 `phosphor-host-ipmid` 的 sensor/SDR 和 FRU 相关路径；识别 IPMI 的模型限制，避免为新功能强行增加不兼容的 OEM raw command。
- SNMP：复盘你已实现的数据路径，补充缓存、D-Bus 失败、设备消失、单位与 SNMP OID 语义的处理。
- 决策规则：标准资产/传感器优先进入稳定 D-Bus 模型，再由标准 Redfish/IPMI 复用；仅产品专有且无标准位置时才考虑 OEM 扩展。

**验收：** 对一个新字段能够说明为什么选择标准 Redfish、IPMI、SNMP OID 或 OEM 扩展，并评估兼容性。

### 第 14 周：测试、构建与代码质量

- 在你的产品 layer 中追一项功能的 bitbake 依赖、recipe、patch 和配置安装位置；理解修改为什么会/不会进入镜像。
- 为一个 parser、配置转换或 D-Bus 映射增加/修改单元测试；若现有项目测试框架不同，以项目既有测试风格为准。
- 制定最小回归清单：冷启动、热插拔（如支持）、设备异常、服务重启、Redfish/IPMI/SNMP 查询、日志验证。
- C++：学习 sanitizers、编译告警、`clang-format`/项目 lint 规则；Python：学习 `pytest`、类型标注和 `ruff` 或项目既有工具。

**验收：** 新功能不只在手工“正常路径”验证过，还能复现并验证至少一个失败路径。

### 第 15–16 周：独立交付一个设备接入

选择当前项目最紧急、边界清晰的一项：建议顺序为 **PSU → 传感器 → GPU → RAID/硬盘**（实际优先级应由产品需求和硬件可访问性决定）。

交付物：

1. 一页设计卡与数据流图；
2. 配置及必要源码改动；
3. D-Bus object/interface/property/association 验证记录；
4. Redfish/IPMI/SNMP 中实际需要的验证记录；
5. 正常、缺失、读失败、异常状态的测试结果；
6. 一个后续改进项列表（例如驱动、阈值策略、事件去抖或协议支持）。

## 5. 新增设备时的通用实施模板

在改代码前，按下表填空。填不出来的项目就是需要先调研或向硬件团队确认的风险。

| 设计项 | 要回答的问题 |
|---|---|
| 设备身份 | 名称、槽位、FRU 来源、型号/序列号/料号从哪里来？ |
| 硬件通道 | I2C/PMBus、GPIO、hwmon、SMBIOS、PCIe、MCTP/PLDM，还是厂商协议？ |
| Presence | 哪个硬件信号或协议结果代表“在位”？是否需要去抖？ |
| Functional | 哪些故障位、读失败或自检结果代表“不正常”？ |
| Sensor | 有哪些量值、单位、采样周期、阈值、无效值？ |
| D-Bus 生产者 | Entity Manager、dbus-sensors、phosphor-hwmon、还是新的专用 daemon？ |
| D-Bus 模型 | Inventory/Sensor/OperationalStatus/Availability/Association 的 object path 与接口是什么？ |
| 消费者 | Redfish、IPMI、SNMP、Web UI、fan/power policy 中哪些需要使用它？ |
| 生命周期 | BMC 启动、Host 未启动、热插拔、服务重启、I2C 超时后分别怎样？ |
| 测试 | 怎样制造在位、缺失、读失败、超阈值、故障和恢复？ |

## 6. 设备类型的推荐起点

| 设备 | 首先确认 | 常见实现组合 | 重点风险 |
|---|---|---|---|
| 电源 PSU | GPIO/PMBus Presence、PMBus 寄存器、FRU | Entity Manager + dbus-sensors/hwmon + Inventory + PowerSupply | 设备在位、输入/输出故障和传感器不可读不是同一状态 |
| 硬盘 | 背板/控制器/Host 管理数据源 | SMBIOS 或 Storage 服务 + Redfish Storage/Drive | BMC 往往不能直接得到完整 SMART/RAID 状态；先确认数据源 |
| RAID 卡 | PCIe/SMBIOS/控制器带外协议 | Inventory + 专用服务 / MCTP/PLDM（若支持） | “控制器存在”和“阵列健康”需分开建模 |
| GPU | PCIe Presence、带外管理接口、温度/功耗来源 | Inventory + dbus-sensors 或专用 vendor 服务 | 不要用静态配置假装动态可管理能力 |
| 板载 Sensor | 驱动、sysfs/hwmon、I2C 地址 | Entity Manager Exposes + dbus-sensors 或 phosphor-hwmon | 单位、缩放系数、阈值、关联对象和异常值处理 |

## 7. C++ 与 Python 学习安排（嵌入式 OpenBMC 导向）

### C++：每周 2–3 小时，跟源码一起学

优先级从高到低：

1. C++11/14 实战：RAII、移动语义、`unique_ptr/shared_ptr`、`optional`、`variant`、lambda、`std::chrono`；
2. 错误与资源边界：异常、`std::error_code`、文件描述符/DBus connection 的生命周期；
3. 异步模型：`boost::asio`、sdbusplus 的异步调用、timer、取消和对象销毁；
4. 可维护性：拆分纯函数、减少全局状态、日志上下文、单元测试、避免阻塞 D-Bus event loop；
5. 阅读方式：每周精读一个 200–400 行的实际功能路径，画出输入、状态、异步回调、输出和错误分支。

**不要脱离 OpenBMC 单独刷大量算法题。** 对当前工作最有回报的是能读懂异步服务、诊断 resource lifetime 和写出可测试的配置/协议转换代码。

### Python：每周 1–2 小时，服务调试和测试自动化导向

1. JSON/YAML 解析、`pathlib`、`argparse`、`subprocess.run`、结构化日志；
2. 通过命令行或适配库采集 D-Bus、Redfish、journal 快照；
3. `pytest` 参数化测试：同一套用例验证多颗 sensor 或多个槽位；
4. 正则与文本处理：解析日志时只提取稳定字段，避免依赖易变的整行文本；
5. 类型标注、清晰的错误报告、超时和重试边界。

建议工具项目：`device_health_snapshot.py`。输入一个 Inventory object 或 Redfish resource，输出其 Presence/Functional、关联 sensor、当前值、阈值状态和相关服务状态；后续把它发展为回归测试工具。

## 8. 核心官方阅读清单

按“先架构、后模块、再规范”的顺序阅读，不必一口气全部读完。

| 主题 | 阅读材料 | 阅读目的 |
|---|---|---|
| 全局架构 | [OpenBMC docs](https://github.com/openbmc/docs)、[接口概览](https://github.com/openbmc/docs/blob/master/architecture/interface-overview.md) | 建立 service 与接口地图 |
| D-Bus 契约 | [phosphor-dbus-interfaces YAML](https://github.com/openbmc/phosphor-dbus-interfaces/tree/master/yaml/xyz/openbmc_project) | 以接口定义，而非猜测属性名 |
| 配置/发现 | [Entity Manager API](https://github.com/openbmc/entity-manager/blob/master/docs/entity_manager_dbus_api.md)、[传感器示例](https://github.com/openbmc/entity-manager/blob/master/docs/my_first_sensors.md) | 理解 JSON、Probe、Exposes 与 reactor |
| Sensor | [dbus-sensors README](https://github.com/openbmc/dbus-sensors/blob/master/README.md)、[Sensor architecture](https://github.com/openbmc/docs/blob/master/architecture/sensor-architecture.md) | 建立 hwmon/D-Bus/association 模型 |
| Redfish | [bmcweb Redfish 支持说明](https://github.com/openbmc/bmcweb/blob/master/docs/Redfish.md)、[bmcweb 开发指南](https://github.com/openbmc/bmcweb/blob/master/DEVELOPING.md) | 了解 schema 目标和实现约束 |
| 构建/服务 | [Yocto development](https://github.com/openbmc/docs/blob/master/yocto-development.md)、当前分支 recipe/service | 确保改动进镜像并正确启动 |
| 规范 | [DMTF Redfish](https://www.dmtf.org/standards/redfish)、[Linux hwmon 文档](https://docs.kernel.org/hwmon/index.html) | 统一对外模型与内核读数语义 |

## 9. 每周节奏与复盘

建议每周采用 60/30/10：

- **60%（5–6 小时）**：当前真实需求的源码走读、配置/代码实现和上机验证；
- **30%（2–3 小时）**：围绕本周问题读官方文档和接口 YAML；
- **10%（约 1 小时）**：补 C++/Python、整理设计卡和排障笔记。

每周五用下面四个问题复盘：

1. 本周新增或修改的 D-Bus object 是谁创建的？谁消费它？
2. 出现异常时，哪个 property 会变化？是否有日志/事件？
3. 哪一段结论来自硬件手册，哪一段来自源码，哪一段来自实际验证？
4. 下周是否能用一个脚本或测试替代重复的手工检查？

## 10. 阶段能力标尺

完成本计划后，以下事项应能独立完成：

- 为新硬件选择“配置驱动、已有服务扩展、专用 userspace daemon 或 kernel driver”的合理入口；
- 从 I2C/hwmon/GPIO/SMBIOS 等来源建立稳定的 Inventory 与 Sensor D-Bus 模型；
- 正确区分并处理 Presence、Availability、Functional、Threshold 和日志事件；
- 建立 Inventory、Sensor、Chassis 间的 Association，使 Redfish 能正确归类和展示；
- 在 Redfish/IPMI/SNMP 中选择合适的标准接口或受控的 OEM 扩展；
- 用 systemd、journal、D-Bus、sysfs/协议读数定位“设备没有显示”或“状态不正确”的跨模块问题；
- 为新功能提供正常与失败路径的最小回归验证。

达到这些能力后，你就从“能完成单点 OpenBMC 修改”进入了“能承担平台设备管理链路”的高级能力过渡阶段。下一阶段再深入 Yocto、内核/驱动、MCTP/PLDM、复杂电源时序和热策略，会更顺畅。
