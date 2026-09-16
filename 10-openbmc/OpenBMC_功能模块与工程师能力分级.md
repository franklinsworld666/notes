# OpenBMC 功能模块与工程师能力分级

> 目标：建立 OpenBMC 功能模块、主要源码项目、功能职责和中级/高级能力要求之间的对应关系。

> **分级定义**：中级 = 能独立阅读、修改和调试；高级 = 能跨模块设计、处理平台差异、深入 Linux/硬件/协议并完成系统级 Debug。

官方架构文档也以 systemd service 为线索描述各服务和接口之间的关系。

## 模块总览

| # | 功能模块 | 主要源码模块/项目 | 一句话简介 | 能力等级 |
|---:|---|---|---|---|
## 一、基础设施与构建系统

|   # | 功能模块                 | 主要源码模块/项目                                   | 功能一句话简介                                              | 能力等级      | 状态  |
| --: | -------------------- | ------------------------------------------- | ---------------------------------------------------- | --------- | --- |
|   1 | OpenBMC Distribution | `openbmc`                                   | 通过 Yocto/OpenEmbedded 组合 layer、recipe 和软件组件生成 BMC 固件 | **中级**    |     |
|   2 | Yocto/OpenEmbedded   | `openembedded-core / BitBake`               | 负责交叉编译、依赖管理、rootfs 和镜像生成                             | **高级**    |     |
|   3 | meta-phosphor        | `meta-phosphor`                             | 提供跨平台通用 OpenBMC recipe、systemd 集成和 feature           | **中级**    |     |
|   4 | SoC Layer            | `meta-aspeed / meta-nuvoton / 其他 SoC layer` | 提供 BMC SoC 的 Kernel、U-Boot、驱动和硬件配置                   | **高级**    |     |
|   5 | Board Layer          | `meta-<vendor> / meta-<board>`              | 提供具体服务器平台的设备树、配置、服务和平台策略                             | **高级**    |     |
|   6 | systemd Integration  | `obmc-phosphor-systemd.bbclass`             | 把 recipe 与 systemd service、target、socket 等启动机制连接起来   | **中级→高级** |     |
|   7 | systemd              | `Linux systemd`                             | 负责服务生命周期、启动依赖、target、timer 和日志                       | **初级**    | 完成  |
|   8 | D-Bus                | `Linux dbus`                                | 提供 OpenBMC 核心进程间通信机制                                 | **中级**    |     |
|   9 | sdbusplus            | `sdbusplus`                                 | 为 C++ 程序提供 D-Bus 方法、属性、信号等封装                         | **中级**    |     |
|  10 | sdeventplus          | `sdeventplus`                               | 将事件循环与 C++ 异步程序结合                                    | **中级→高级** |     |
|  11 | D-Bus Interfaces     | `phosphor-dbus-interfaces`                  | 定义跨服务共享的标准 D-Bus API                                 | **中级**    |     |
|  12 | Object Mapper        | `phosphor-objmgr`                           | 提供 D-Bus service/object/interface 的发现与映射             | **中级**    |     |
|  13 | D-Bus Monitor        | `phosphor-dbus-monitor`                     | 根据 D-Bus 对象和属性变化触发动作                                 | **中级**    |     |
## 二、硬件访问与 Linux 基础设施

|   # | 功能模块          | 主要源码模块/项目                          | 功能一句话简介                                       | 能力等级      |
| --: | ------------- | ---------------------------------- | --------------------------------------------- | --------- |
|   1 | I2C           | `Linux I2C subsystem / i2c-tools`  | 提供 BMC 与 EEPROM、CPLD、Sensor、VRM 等器件通信         | **中级**    |
|   2 | GPIO          | `Linux GPIO / libgpiod / gpioplus` | 提供 GPIO 输入、输出和边沿事件处理                          | **中级**    |
|   3 | SPI           | `Linux SPI subsystem / spidev`     | 提供 BMC 与 SPI Flash、CPLD 等设备通信                 | **中级**    |
|   4 | EEPROM        | `Linux EEPROM / platform code`     | 管理 FRU、配置等非易失数据                               | **中级**    |
|   5 | HWMON         | `Linux hwmon`                      | 向 userspace 暴露温度、电压、电流、风扇等监控数据                | **中级**    |
|   6 | PWM           | `Linux PWM`                        | 提供风扇等设备的 PWM 控制                               | **中级**    |
|   7 | RTC           | `Linux RTC`                        | 提供硬件实时时钟访问                                    | **中级**    |
|   8 | MTD           | `Linux MTD`                        | 管理 SPI-NOR/NAND 等 Flash                       | **高级**    |
|   9 | UBI/UBIFS     | `Linux UBI/UBIFS`                  | 提供 NAND 坏块管理和 Flash 文件系统                      | **高级**    |
|  10 | Linux Network | `Linux networking`                 | 提供 Ethernet、IPv4/IPv6、VLAN、route、socket 等底层能力 | **中级→高级** |
|  11 | USB Gadget    | `Linux USB Gadget`                 | 让 BMC 以 USB device 方式向 Host 提供虚拟设备            | **高级**    |
|  12 | PCIe          | `Linux PCI subsystem`              | 管理 PCIe 设备及资源                                 | **高级**    |
|  13 | PECI          | `Linux PECI / libpeci`             | 提供 BMC 与 Intel CPU 的管理通信                      | **高级**    |
## 三、Inventory / FRU / SMBIOS

|   # | 功能模块                   | 主要源码模块/项目                            | 功能一句话简介                               | 能力等级      | 状态  |
| --: | ---------------------- | ------------------------------------ | ------------------------------------- | --------- | --- |
|   1 | Entity Manager         | `entity-manager`                     | 根据 JSON Probe/配置动态发现并创建平台实体及 D-Bus 对象 | **中级→高级** |     |
|   2 | FRU Device             | `phosphor-fru-device`                | 从 FRU EEPROM 读取资产数据并转换为 D-Bus 信息      | **中级**    |     |
|   3 | IPMI FRU               | `phosphor-ipmi-fru`                  | 实现 FRU 数据解析、构造和访问                     | **中级→高级** | 完成  |
|   5 | Inventory Manager      | `phosphor-inventory-manager`         | 管理硬件 Inventory 对象及生命周期                | **中级**    |     |
|   6 | SMBIOS                 | `smbios-mdr`                         | 获取 Host SMBIOS 并提供平台信息                | **中级→高级** |     |
|   7 | Inventory Associations | `D-Bus associations / platform code` | 建立实体、传感器、错误和 FRU 的关联                  | **高级**    |     |
## 四、Sensor

|   # | 功能模块               | 主要源码模块/项目                         | 功能一句话简介                            | 能力等级      |
| --: | ------------------ | --------------------------------- | ---------------------------------- | --------- |
|   1 | dbus-sensors       | `dbus-sensors`                    | 把多种硬件传感器统一转换为 D-Bus Sensor         | **中级**    |
|   2 | phosphor-hwmon     | `phosphor-hwmon`                  | 从 Linux hwmon/sysfs 获取传感器并发布 D-Bus | **中级**    |
|   3 | Virtual Sensor     | `phosphor-virtual-sensor`         | 根据多个基础传感器计算虚拟值                     | **中级**    |
|   4 | Sensor Threshold   | `dbus-sensors / D-Bus interfaces` | 管理 Warning/Critical 等阈值和状态         | **中级**    |
|   5 | Sensor Association | `D-Bus associations`              | 建立 Sensor 与实体/FRU 的逻辑关系            | **中级→高级** |
|   6 | Sensor Discovery   | `dbus-sensors / Entity Manager`   | 根据平台配置发现实际传感器                      | **中级→高级** |
## 五、Power / Host / Chassis

|   # | 功能模块              | 主要源码模块/项目                                 | 功能一句话简介                             | 能力等级      |
| --: | ----------------- | ----------------------------------------- | ----------------------------------- | --------- |
|   1 | BMC State         | `phosphor-state-manager`                  | 管理 BMC Ready、NotReady、Rebooting 等状态 | **中级**    |
|   2 | Host State        | `phosphor-state-manager`                  | 管理 Host Off、Running、Quiesced 等状态    | **中级→高级** |
|   4 | Host Control      | `obmc-host-ctl`                           | 提供 Host 启动、停止、重启等控制入口               | **中级**    |
|   5 | Power Control     | `phosphor-power / platform power control` | 根据平台策略控制服务器电源                       | **中级→高级** |
|   6 | x86 Power Control | `x86-power-control`                       | 实现 x86 CPU/Host 电源时序和状态控制           | **高级**    |
|   7 | Power Sequencing  | `platform-specific`                       | 使用 GPIO/I2C/CPLD 完成实际电源时序           | **高级**    |
|   8 | Host Reset        | `platform code`                           | 控制 Host reset 并协调 BMC/Host 状态机      | **高级**    |
## 六、Fan / Thermal

| # | 功能模块 | 主要源码模块/项目 | 功能一句话简介 | 能力等级 |
|---:|---|---|---|---|
| 1 | Fan Presence | `phosphor-fan-presence` | 检测风扇是否存在 | **中级** |
| 2 | Fan Sensor | `dbus-sensors` | 获取风扇 Tach/RPM 数据 | **中级** |
| 3 | Fan Control | `phosphor-fan-control` | 根据平台策略控制风扇 | **中级→高级** |
| 4 | PID Control | `phosphor-pid-control` | 根据温度等输入计算风扇控制量 | **高级** |
| 5 | Fan Monitor | `phosphor-fan-monitor` | 监测风扇低速、停止和故障 | **中级** |
| 6 | Thermal Policy | `platform policy` | 根据多传感器和硬件约束制定散热策略 | **高级** |
## 七、LED / GPIO / Button

| # | 功能模块 | 主要源码模块/项目 | 功能一句话简介 | 能力等级 |
|---:|---|---|---|---|
| 1 | LED Manager | `phosphor-led-manager` | 管理逻辑 LED 状态和组态 | **中级** |
| 2 | LED Sysfs | `phosphor-led-sysfs` | 把逻辑 LED 映射到 Linux LED/sysfs | **中级** |
| 3 | LED Fault Monitor | `phosphor-led-manager-faultmonitor` | 根据系统故障自动设置故障 LED | **中级** |
| 4 | GPIO Monitor | `phosphor-gpio-monitor` | 把 GPIO 状态/边沿转换成系统事件 | **中级** |
| 5 | Button | `phosphor-buttons` | 处理物理按键输入和系统动作 | **中级** |
## 八、Logging / SEL / Debug

|   # | 功能模块            | 主要源码模块/项目                           | 功能一句话简介                   | 能力等级      | 状态  |
| --: | --------------- | ----------------------------------- | ------------------------- | --------- | --- |
|   1 | Error Logging   | `phosphor-logging`                  | 提供统一错误记录和错误元数据            | **中级**    |     |
|   2 | Event Log       | `phosphor-logging`                  | 保存和管理系统运行事件               | **中级**    |     |
|   3 | IPMI SEL        | `phosphor-sel-logger`               | 将系统事件与 IPMI SEL 模型结合      | **中级→高级** |     |
|   4 | Journal         | `systemd-journald`                  | 保存服务和系统日志                 | **中级**    | 完成  |
|   5 | Remote Logging  | `rsyslog / phosphor-rsyslog-config` | 把 BMC 日志转发到远端服务器          | **中级**    |     |
|   6 | Debug Collector | `phosphor-debug-collector`          | 收集日志、配置和状态用于故障诊断          | **中级→高级** |     |
|   7 | Crash Dump      | `platform-specific / kernel`        | 收集 Kernel 或 Host 异常后的诊断信息 | **高级**    |     |
## 九、Network / Time / SNMP / LDAP

| # | 功能模块 | 主要源码模块/项目 | 功能一句话简介 | 能力等级 |
|---:|---|---|---|---|
| 1 | Network Manager | `phosphor-networkd` | 通过 D-Bus 管理 BMC 网络配置 | **中级** |
| 2 | Ethernet | `phosphor-networkd / Linux` | 管理网卡、MAC 和链路 | **中级** |
| 3 | IPv4 | `phosphor-networkd / Linux` | 管理 IPv4 地址和路由 | **中级** |
| 4 | IPv6 | `phosphor-networkd / Linux` | 管理 IPv6 地址、RA 和路由 | **高级** |
| 5 | DHCP | `systemd-networkd / platform` | 为 BMC 获取动态网络配置 | **中级** |
| 6 | VLAN | `phosphor-networkd / Linux` | 管理 BMC VLAN interface | **中级→高级** |
| 7 | Routing | `Linux networking` | 管理多网络环境下的路由 | **高级** |
| 8 | DNS | `systemd/network stack` | 管理域名解析配置 | **中级** |
| 9 | NTP | `phosphor-time-manager` | 管理 BMC 时间同步策略和 NTP | **中级** |
| 10 | RTC | `Linux RTC / time manager` | 管理硬件时钟与系统时间关系 | **中级** |
| 11 | SNMP | `phosphor-snmp` | 提供 SNMP 管理和 Trap | **中级** |
| 12 | LDAP | `phosphor-ldap` | 把企业 LDAP 接入 BMC 认证 | **高级** |
| 13 | mDNS | `Avahi / platform` | 提供局域网服务发现 | **中级** |
## 十、IPMI

|   # | 功能模块          | 主要源码模块/项目                                | 功能一句话简介                        | 能力等级      | 状态  |
| --: | ------------- | ---------------------------------------- | ------------------------------ | --------- | --- |
|   1 | IPMI Core     | `phosphor-host-ipmid`                    | 提供 IPMI command framework      | **中级**    |     |
|   2 | IPMI Provider | `phosphor-host-ipmid`                    | 把具体 IPMI command 映射到平台功能       | **中级→高级** |     |
|   3 | Host IPMI     | `phosphor-host-ipmid`                    | 处理 Host 与 BMC 的 in-band IPMI   | **高级**    |     |
|   4 | Network IPMI  | `phosphor-net-ipmid / phosphor-ipmi-net` | 提供网络侧 IPMI/RMCP+               | **高级**    |     |
|   5 | IPMB          | `ipmbbridge / phosphor-ipmb-bridge`      | 通过 IPMB 与外部管理控制器通信             | **高级**    |     |
|   6 | KCS           | `kcsbridge / host IPMI`                  | 提供 Host-BMC KCS 通道             | **高级**    |     |
|   7 | SSIF          | `ssifbridge / platform`                  | 通过 SMBus/SSIF 提供 Host-BMC IPMI | **高级**    |     |
|   8 | BT            | `btbridge / platform`                    | 提供 Host-BMC BT 通道              | **高级**    |     |
|   9 | IPMI FRU      | `phosphor-ipmi-host`                     | 通过 IPMI 暴露 FRU                 | **中级**    | 完成  |
|  10 | IPMI Sensor   | `phosphor-host-ipmid`                    | 把 D-Bus Sensor 转换成 IPMI Sensor | **中级→高级** |     |
|  11 | IPMI SEL      | `phosphor-sel-logger`                    | 提供 IPMI System Event Log       | **中级**    |     |
|  12 | IPMI OEM      | `phosphor-host-ipmid plugins / platform` | 实现厂商自定义 IPMI command           | **高级**    |     |
|  13 | ipmitool      | `ipmitool`                               | 实现 ipmitool 工具                 | **中级**    |     |
## 十一、Redfish / Web

| # | 功能模块 | 主要源码模块/项目 | 功能一句话简介 | 能力等级 |
|---:|---|---|---|---|
| 1 | Redfish Server | `bmcweb` | OpenBMC 核心 Redfish/HTTPS 服务 | **中级→高级** |
| 2 | Redfish Schema | `bmcweb` | 把 OpenBMC 数据模型映射成 DMTF Redfish schema | **高级** |
| 3 | Redfish D-Bus Mapping | `bmcweb` | 把 Redfish 请求转换成 D-Bus 操作 | **高级** |
| 4 | Authentication | `bmcweb / PAM` | 实现 Web 用户认证 | **高级** |
| 5 | Authorization | `bmcweb / privilege model` | 控制不同用户的 API 权限 | **高级** |
| 6 | WebSocket | `bmcweb` | 提供实时网络通信通道 | **高级** |
| 7 | REST API | `bmcweb` | 提供 OpenBMC REST 风格接口 | **中级** |
| 8 | Web UI | `webui-vue` | 提供浏览器端 BMC 管理界面 | **中级** |
| 9 | KVM Web Integration | `bmcweb / obmc-ikvm` | 通过 WebSocket 等机制提供远程 KVM | **高级** |
| 10 | Virtual Media Web Integration | `bmcweb / USB-IP` | 通过网络管理 Host 虚拟介质 | **高级** |
## 十二、Console / KVM / Virtual Media

|   # | 功能模块          | 主要源码模块/项目                    | 功能一句话简介                                      | 能力等级   | 状态  |
| --: | ------------- | ---------------------------- | -------------------------------------------- | ------ | --- |
|   1 | Host Console  | `obmc-console`               | 把 Host UART console 抽象成 BMC 可访问的 Unix socket | **中级** |     |
|   2 | SOL           | `obmc-console + IPMI`        | 把 Host Serial Console 通过网络提供给远程用户            | **高级** |     |
|   3 | SSH Console   | `obmc-console`               | 通过 SSH 连接 BMC Console                        | **中级** |     |
|   4 | KVM           | `obmc-ikvm`                  | 把 Host framebuffer/input 转换成远程 KVM           | **高级** |     |
|   5 | Virtual Media | `bmcweb / USB-IP / platform` | 把远程 ISO/媒体映射成 Host 虚拟 USB                    | **高级** |     |
|   6 | USB Gadget    | `Linux USB Gadget`           | 为 Virtual Media 等功能提供 USB device 能力          | **高级** |     |
## 十三、Software / Firmware Update

|   # | 功能模块                   | 主要源码模块/项目                         | 功能一句话简介                    | 能力等级      | 状态  |
| --: | ---------------------- | --------------------------------- | -------------------------- | --------- | --- |
|   1 | Software Manager       | `phosphor-bmc-code-mgmt`          | 管理 BMC 软件版本和软件对象           | **中级→高级** |     |
|   2 | Firmware Upload        | `phosphor-bmc-code-mgmt / bmcweb` | 接收并验证 BMC firmware image   | **高级**    |     |
|   3 | Firmware Activation    | `phosphor-bmc-code-mgmt`          | 控制新 firmware 激活            | **高级**    |     |
|   4 | Firmware Rollback      | `platform-specific`               | 升级失败时恢复可工作的 firmware       | **高级**    |     |
|   5 | Host Firmware Update   | `platform-specific / Redfish`     | 管理 BIOS/Host firmware 更新   | **高级**    |     |
|   6 | Flash Management       | `platform code / MTD`             | 管理 Flash 分区、读写和镜像布局        | **高级**    |     |
|   7 | Secure Firmware Update | `platform/security`               | 对 firmware 做签名验证、版本控制和安全激活 | **高级**    |     |
## 十四、Watchdog / Health / Telemetry

|   # | 功能模块                   | 主要源码模块/项目                         | 功能一句话简介                | 能力等级      |
| --: | ---------------------- | --------------------------------- | ---------------------- | --------- |
|   1 | BMC Watchdog           | `phosphor-watchdog`               | 在 BMC 软件失去响应时执行恢复      | **中级**    |
|   2 | Host Watchdog          | `IPMI / D-Bus / platform`         | 监控 Host 并触发恢复          | **高级**    |
|   3 | Health Monitor         | `phosphor-health-monitor`         | 综合状态判断 BMC/平台健康度       | **高级**    |
|   4 | Telemetry              | `OpenBMC telemetry components`    | 收集并输出平台运行指标            | **中级→高级** |
|   5 | Performance Monitoring | `Linux perf / tracing / platform` | 分析 CPU、内存、I/O、D-Bus 性能 | **高级**    |
## 十五、Security / User Management

| # | 功能模块 | 主要源码模块/项目 | 功能一句话简介 | 能力等级 |
|---:|---|---|---|---|
| 1 | User Manager | `phosphor-user-manager` | 管理 BMC 用户、密码和账户生命周期 | **中级** |
| 2 | PAM | `Linux PAM` | 提供本地及外部认证框架 | **高级** |
| 3 | LDAP Authentication | `phosphor-ldap` | 把 LDAP 用户目录接入 BMC 登录认证 | **高级** |
| 4 | Role / Privilege | `D-Bus / bmcweb / IPMI` | 控制不同用户可执行的管理操作 | **高级** |
| 5 | Certificate Manager | `phosphor-certificate-manager` | 管理 HTTPS/TLS 等数字证书 | **高级** |
| 6 | TLS | `OpenSSL / bmcweb` | 提供加密网络通信 | **高级** |
| 7 | TPM | `Linux TPM / OpenBMC TPM components` | 提供硬件可信能力 | **高级** |
| 8 | Secure Boot | `U-Boot / Kernel / platform` | 建立从 Bootloader 到系统镜像的可信启动链 | **高级** |
| 9 | Security Hardening | `Yocto / Linux / platform` | 通过最小权限和隔离降低 BMC 攻击面 | **高级** |
## 十六、MCTP / PLDM / SPDM / DMTF PMCI

|   # | 功能模块                 | 主要源码模块/项目                 | 功能一句话简介                       | 能力等级   |
| --: | -------------------- | ------------------------- | ----------------------------- | ------ |
|   1 | MCTP                 | `mctp / Linux MCTP`       | 提供管理控制器与设备之间的标准消息传输           | **高级** |
|   2 | libpldm              | `libpldm`                 | 提供 PLDM 编解码和公共库               | **高级** |
|   3 | PLDM Platform        | `pldm`                    | 使用 PLDM 管理平台状态和监控             | **高级** |
|   4 | PLDM FRU             | `pldm / libpldm`          | 使用 PLDM 管理设备 FRU              | **高级** |
|   5 | PLDM Firmware Update | `pldm`                    | 通过 PLDM 更新设备固件                | **高级** |
|   6 | PLDM BIOS            | `pldm`                    | 使用 PLDM 管理 BIOS/Host firmware | **高级** |
|   7 | SPDM                 | `OpenBMC SPDM components` | 提供设备身份认证、测量和安全通信              | **高级** |
|   8 | DMTF PMCI            | `OpenBMC PMCI components` | 实现 DMTF PMCI 设备管理协议           | **高级** |
|   9 | PCIe MCTP            | `Linux MCTP / platform`   | 在 PCIe 管理网络上运行 MCTP           | **高级** |
## 十七、CPU / Memory / PCIe / Platform Management

| # | 功能模块 | 主要源码模块/项目 | 功能一句话简介 | 能力等级 |
|---:|---|---|---|---|
| 1 | CPU Management | `x86-power-control / PECI` | 管理 CPU 状态、温度、电源和平台信息 | **高级** |
| 2 | CPU Thermal | `PECI / sensor stack` | 获取 CPU 温度并参与 Thermal Policy | **高级** |
| 3 | Memory Inventory | `SMBIOS / D-Bus` | 获取 DIMM 容量、速度、序列号等信息 | **中级→高级** |
| 4 | Memory Monitoring | `platform-specific` | 监控 DIMM 温度、ECC 等状态 | **高级** |
| 5 | VRM Management | `I2C / hwmon / platform` | 管理 CPU/Memory/Board VRM 电压、电流和状态 | **高级** |
| 6 | CPLD Management | `platform-specific` | 通过 I2C/SPI 管理 CPLD 寄存器、版本和控制 | **高级** |
| 7 | FPGA Management | `platform-specific` | 管理 FPGA 配置、状态和固件 | **高级** |
| 8 | PCIe Device Management | `Linux PCI / MCTP / PLDM` | 发现并管理 PCIe 设备及管理协议 | **高级** |
| 9 | PCIe Retimer | `platform-specific / MCTP` | 管理 PCIe Retimer 状态、配置和固件 | **高级** |
## 十八、Bootloader / Kernel / Platform Bring-up

| # | 功能模块 | 主要源码模块/项目 | 功能一句话简介 | 能力等级 |
|---:|---|---|---|---|
| 1 | U-Boot | `u-boot / OpenBMC U-Boot layer` | 完成 BMC 启动初始化和 Kernel 加载 | **高级** |
| 2 | Device Tree | `Linux DTS / SoC layer` | 描述 SoC、总线、设备和平台硬件拓扑 | **高级** |
| 3 | Linux Kernel | `OpenBMC Kernel layer` | 提供设备驱动、网络、文件系统和硬件抽象 | **高级** |
| 4 | Driver Model | `Linux Kernel` | 管理 device、driver、bus 和 probe 生命周期 | **高级** |
| 5 | I2C Driver | `Linux Kernel I2C` | 把 I2C 控制器和设备接入 Linux device model | **高级** |
| 6 | GPIO Driver | `Linux GPIO` | 把 GPIO controller 和 events 提供给 userspace | **高级** |
| 7 | HWMON Driver | `Linux HWMON` | 把监控芯片接入 hwmon 框架 | **高级** |
| 8 | SPI Driver | `Linux SPI` | 把 SPI controller/device 接入 Linux | **高级** |
| 9 | MTD Driver | `Linux MTD` | 把 BMC Flash 接入 Linux 存储框架 | **高级** |
| 10 | Network Driver | `Linux network driver` | 为 BMC Ethernet 控制器提供 Linux 网络支持 | **高级** |
| 11 | Kernel Debug | `Linux kernel tools` | 使用 dmesg/ftrace/dynamic debug/crash 定位内核问题 | **高级** |
| 12 | Platform Bring-up | `SoC + board layer + kernel` | 从新硬件启动到完整 OpenBMC 服务完成平台 bring-up | **高级** |
## 十九、测试 / CI / 工程质量

|   # | 功能模块                 | 主要源码模块/项目                         | 功能一句话简介                            | 能力等级      |
| --: | -------------------- | --------------------------------- | ---------------------------------- | --------- |
|   1 | Unit Test            | `各项目 test`                        | 验证单个类、函数或组件行为                      | **中级**    |
|   2 | D-Bus Test           | `sdbusplus / project tests`       | 验证 D-Bus method/property/signal 行为 | **中级**    |
|   3 | Integration Test     | `OpenBMC test infrastructure`     | 验证多个服务组成的完整功能链路                    | **高级**    |
|   4 | QEMU                 | `OpenBMC QEMU support`            | 没有真实硬件时运行和测试 OpenBMC               | **中级→高级** |
|   5 | Redfish Testing      | `Redfish Validator / test suites` | 验证 Redfish API 规范兼容性               | **高级**    |
|   6 | IPMI Testing         | `ipmitool / test suites`          | 验证 IPMI command 和平台行为              | **中级**    |
|   7 | Automated Testing    | `OpenBMC tests`                   | 自动执行系统级回归测试                        | **高级**    |
|   8 | CI                   | `Jenkins / project CI`            | 对提交代码执行编译、测试和质量检查                  | **高级**    |
|   9 | Static Analysis      | `compiler warnings / clang tools` | 在运行前发现潜在错误和质量问题                    | **中级**    |
|  10 | Performance Analysis | `perf / tracing`                  | 分析 CPU、memory、I/O 和 latency        | **高级**    |
|  11 | Code Review          | `Gerrit / GitHub`                 | 通过代码审查保证设计、质量和兼容性                  | **中级→高级** |

## 高级工程师最重要的跨模块能力

| 能力 | 典型问题 | 等级 |
|---|---|---|
| D-Bus 架构 | 一个新硬件属性应该放在哪个 object/interface？ | 高级 |
| Hardware → D-Bus | 硬件数据如何抽象成稳定的数据模型？ | 高级 |
| D-Bus → Redfish | 如何把内部模型映射为 Redfish schema？ | 高级 |
| D-Bus → IPMI | 如何兼容旧 IPMI 接口？ | 高级 |
| I2C → Kernel → userspace | 应该写 driver 还是直接访问 `/dev/i2c-*`？ | 高级 |
| systemd 架构 | 服务依赖、启动时序和恢复策略如何设计？ | 高级 |
| FRU 架构 | EEPROM 数据如何与 Inventory 对齐？ | 高级 |
| Sensor architecture | Sensor 应该由哪个服务创建和维护？ | 高级 |
| Power architecture | GPIO/CPLD/State Manager 如何协作？ | 高级 |
| Thermal architecture | Sensor/PID/PWM/Fan 如何形成闭环？ | 高级 |
| Firmware update | 如何保证升级失败后仍可恢复？ | 高级 |
| Platform bring-up | 新板卡如何从硬件 bring-up 到完整 OpenBMC？ | 高级 |
| Protocol selection | IPMI / Redfish / MCTP / PLDM 如何选择？ | 高级 |
| Performance | D-Bus、CPU、I/O latency 从哪里来？ | 高级 |
| Security | 如何降低 BMC attack surface？ | 高级 |

## 判断自己是否达到高级水平

不要用“我看过多少源码”判断，而要用下面的问题判断：

1. 给你一个新 CPLD，你能否决定应该在 userspace 直接访问还是增加 Kernel driver？
2. 给你一个新 Sensor，你能否设计从硬件到 D-Bus、IPMI、Redfish 的完整链路？
3. 给你一个新的 FRU 字段，你能否判断应该改 FRU parser、Entity Manager、Inventory、IPMI 还是 bmcweb？
4. 给你一个 `ipmitool raw` 命令失败的问题，你能否从 transport 一直追到硬件？
5. 给你一个 Redfish property，你能否找到对应的 D-Bus object/interface/property？
6. 给你一个“系统开机偶发失败”的问题，你能否结合 systemd、journal、D-Bus、GPIO/I2C 和 Kernel 日志定位？
7. 给你一块新服务器板卡，你能否参与从 U-Boot、Device Tree、Kernel、硬件访问、D-Bus 到 Redfish 的完整 bring-up？

如果这些问题能够独立解决，你的能力就已经明显超过“会修改某一个 OpenBMC 模块”的工程师。

## 当前水平

### 服务器/BMC 厂商信息

+ 信息源：FRU
+ 如何加载/卸载 eeprom 驱动
+ 如何读取和解析/校验 fru 数据，并发布到 dbus
+ 开发了 python 脚本解析和校验 fru
+ ipmitool 命令查询、编辑、写入 fru

### Bios 提供的硬件信息

+ 信息源：smbios
+ 如何解析 smbios 表，开发了 python 解析脚本
+ 

