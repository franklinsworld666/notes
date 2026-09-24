# SMBIOS 格式介绍

## 1. 简介

SMBIOS（System Management BIOS）是由 DMTF 定义的固件数据规范，用于向操作系统和管理控制器描述系统硬件、固件及资产信息。SMBIOS 数据通常由 BIOS/UEFI 在主机启动阶段生成。

SMBIOS 常被称为 DMI 信息。实践中，Linux 的 `dmidecode`、BMC 资产管理服务及带外管理接口都可使用 SMBIOS 数据。

本文中各 Type 的结构格式以 [DMTF SMBIOS 3.5.0（DSP0134）](https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.5.0.pdf) 为准。

## 2. 规范与版本

SMBIOS 主要分为 2.x 与 3.x 两代入口点格式：

- SMBIOS 2.x 使用 32 位结构表地址，结构表最大长度为 64 KiB。
    
- SMBIOS 3.x 使用 64 位结构表地址，适用于更大的结构表。
    
- SMBIOS 结构体本身仍由 `Type`、`Length`、`Handle` 和类型相关字段组成。
    

解析时必须以每个结构体的 `Length` 为边界。某个 Type 在新版本中增加字段时，旧平台提供的结构体可能较短；解析程序不得访问 `Length` 之外的字段。

## 3. SMBIOS 数据组织

```
+---------------------+
| SMBIOS Entry Point  |
+---------------------+
           |
           v
+---------------------+
| SMBIOS Structure    |
| Table               |
|  - Type 0           |
|  - Type 1           |
|  - Type 2           |
|  - ...              |
|  - Type 127         |
+---------------------+
```

### 3.1 Entry Point

入口点包含 SMBIOS 版本、结构表地址和结构表大小等元数据。

- SMBIOS 2.x 入口点锚点为 `_SM_`
    
- SMBIOS 3.x 入口点锚点为 `_SM3_`
    

### 3.2 结构体通用头部

每个 SMBIOS 结构体的前 4 字节均为通用头部：

|偏移|长度|字段|说明|
|---|---|---|---|
| `0x00` |BYTE|Type|结构类型|
| `0x01` |BYTE|Length|格式化区域长度，不包括字符串区域|
| `0x02` |WORD|Handle|此结构体的唯一标识符|

SMBIOS 的多字节数值默认采用**小端序**。

### 3.3 字符串区域

格式化区域之后紧跟字符串区域。字符串字段保存的是字符串索引，而非字符串地址。

```
+---------------------------+
| Formatted Area            |
| Type | Length | Handle    |
| ... string-index fields   |
+---------------------------+
| String 1 \0               |
| String 2 \0               |
| ...                       |
| \0                        |
+---------------------------+
```

- 索引 `1` 表示第一个字符串。
    
- 索引 `0` 表示该字段未提供。
    
- 连续两个 `00` 字节表示当前结构体结束。
    
- SMBIOS 3.5 规定字符串编码为 UTF-8。
    

## 4. 常见结构类型及 SMBIOS 3.5 格式

下列表中的偏移均相对于结构体起始地址。`BYTE`、`WORD`、`DWORD`、`QWORD` 分别表示 1、2、4、8 字节无符号整数；`STRING` 表示一个字符串索引。

### 4.1 Type 0：BIOS Information

Type 0 描述 BIOS 或 UEFI 固件信息。

| 偏移     | 长度     | 字段                                         |
| ------ | ------ | ------------------------------------------ |
| `0x00` | BYTE   | Type，固定为 `0`                               |
| `0x01` | BYTE   | Length                                     |
| `0x02` | WORD   | Handle                                     |
| `0x04` | STRING | Vendor                                     |
| `0x05` | STRING | BIOS Version                               |
| `0x06` | WORD   | BIOS Starting Address Segment              |
| `0x08` | STRING | BIOS Release Date                          |
| `0x09` | BYTE   | BIOS ROM Size                              |
| `0x0A` | QWORD  | BIOS Characteristics                       |
| `0x12` | BYTE   | BIOS Characteristics Extension Byte 1      |
| `0x13` | BYTE   | BIOS Characteristics Extension Byte 2      |
| `0x14` | BYTE   | System BIOS Major Release                  |
| `0x15` | BYTE   | System BIOS Minor Release                  |
| `0x16` | BYTE   | Embedded Controller Firmware Major Release |
| `0x17` | BYTE   | Embedded Controller Firmware Minor Release |
| `0x18` | WORD   | Extended BIOS ROM Size                     |

在 SMBIOS 3.5 中，完整 Type 0 的格式化区域长度为 `0x1A`。

### 4.2 Type 1：System Information

Type 1 描述整机标识信息。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `1` |
| `0x01` |BYTE|Length|
| `0x02` |WORD|Handle|
| `0x04` |STRING|Manufacturer|
| `0x05` |STRING|Product Name|
| `0x06` |STRING|Version|
| `0x07` |STRING|Serial Number|
| `0x08` |16 BYTE|UUID|
| `0x18` |BYTE|Wake-up Type|
| `0x19` |STRING|SKU Number|
| `0x1A` |STRING|Family|

完整 Type 1 的格式化区域长度为 `0x1B`。

### 4.3 Type 2：Baseboard Information

Type 2 描述主板或基板信息。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `2` |
| `0x01` |BYTE|Length|
| `0x02` |WORD|Handle|
| `0x04` |STRING|Manufacturer|
| `0x05` |STRING|Product|
| `0x06` |STRING|Version|
| `0x07` |STRING|Serial Number|
| `0x08` |STRING|Asset Tag|
| `0x09` |BYTE|Feature Flags|
| `0x0A` |STRING|Location In Chassis|
| `0x0B` |WORD|Chassis Handle|
| `0x0D` |BYTE|Board Type|
| `0x0E` |BYTE|Number Of Contained Object Handles|
| `0x0F` |WORD × N|Contained Object Handles|

其中 `N` 由 `Number Of Contained Object Handles` 决定，因此 Type 2 的格式化区域长度可变：

```
Length = 0x0F + 2 × N
```

### 4.4 Type 3：System Enclosure or Chassis

Type 3 描述机箱或设备外壳。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `3` |
| `0x01` |BYTE|Length|
| `0x02` |WORD|Handle|
| `0x04` |STRING|Manufacturer|
| `0x05` |BYTE|Type|
| `0x06` |STRING|Version|
| `0x07` |STRING|Serial Number|
| `0x08` |STRING|Asset Tag Number|
| `0x09` |BYTE|Boot-up State|
| `0x0A` |BYTE|Power Supply State|
| `0x0B` |BYTE|Thermal State|
| `0x0C` |BYTE|Security Status|
| `0x0D` |DWORD|OEM-defined|
| `0x11` |BYTE|Height|
| `0x12` |BYTE|Number Of Power Cords|
| `0x13` |BYTE|Contained Element Count|
| `0x14` |BYTE|Contained Element Record Length|
| `0x15` |BYTE × N|Contained Elements|
| `0x15 + N` |STRING|SKU Number|

其中：

```
N = Contained Element Count × Contained Element Record Length
```

SKU Number 位于全部 Contained Elements 之后，因此其偏移是可变的。

### 4.5 Type 4：Processor Information

Type 4 描述单个处理器插槽或处理器实例。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `4` |
| `0x01` |BYTE|Length|
| `0x02` |WORD|Handle|
| `0x04` |STRING|Socket Designation|
| `0x05` |BYTE|Processor Type|
| `0x06` |BYTE|Processor Family|
| `0x07` |STRING|Processor Manufacturer|
| `0x08` |QWORD|Processor ID|
| `0x10` |STRING|Processor Version|
| `0x11` |BYTE|Voltage|
| `0x12` |WORD|External Clock|
| `0x14` |WORD|Max Speed|
| `0x16` |WORD|Current Speed|
| `0x18` |BYTE|Status|
| `0x19` |BYTE|Processor Upgrade|
| `0x1A` |WORD|L 1 Cache Handle|
| `0x1C` |WORD|L 2 Cache Handle|
| `0x1E` |WORD|L 3 Cache Handle|
| `0x20` |STRING|Serial Number|
| `0x21` |STRING|Asset Tag|
| `0x22` |STRING|Part Number|
| `0x23` |BYTE|Core Count|
| `0x24` |BYTE|Core Enabled|
| `0x25` |BYTE|Thread Count|
| `0x26` |WORD|Processor Characteristics|
| `0x28` |WORD|Processor Family 2|
| `0x2A` |WORD|Core Count 2|
| `0x2C` |WORD|Core Enabled 2|
| `0x2E` |WORD|Thread Count 2|

完整 Type 4 的格式化区域长度为 `0x30`。当核心数、启用核心数或线程数超过 `0xFF` 时，应使用对应的扩展字段 `Core Count 2`、`Core Enabled 2`、`Thread Count 2`。

### 4.6 Type 16：Physical Memory Array

Type 16 描述一组物理内存设备所属的内存阵列。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `16` |
| `0x01` |BYTE|Length|
| `0x02` |WORD|Handle|
| `0x04` |BYTE|Location|
| `0x05` |BYTE|Use|
| `0x06` |BYTE|Memory Error Correction|
| `0x07` |DWORD|Maximum Capacity|
| `0x0B` |WORD|Memory Error Information Handle|
| `0x0D` |WORD|Number Of Memory Devices|
| `0x0F` |QWORD|Extended Maximum Capacity|

完整 Type 16 的格式化区域长度为 `0x17`。

### 4.7 Type 17：Memory Device

Type 17 描述一条内存、一个 DIMM 插槽或板载内存设备。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `17` |
| `0x01` |BYTE|Length|
| `0x02` |WORD|Handle|
| `0x04` |WORD|Physical Memory Array Handle|
| `0x06` |WORD|Memory Error Information Handle|
| `0x08` |WORD|Total Width|
| `0x0A` |WORD|Data Width|
| `0x0C` |WORD|Size|
| `0x0E` |BYTE|Form Factor|
| `0x0F` |BYTE|Device Set|
| `0x10` |STRING|Device Locator|
| `0x11` |STRING|Bank Locator|
| `0x12` |BYTE|Memory Type|
| `0x13` |WORD|Type Detail|
| `0x15` |WORD|Speed|
| `0x17` |STRING|Manufacturer|
| `0x18` |STRING|Serial Number|
| `0x19` |STRING|Asset Tag|
| `0x1A` |STRING|Part Number|
| `0x1B` |BYTE|Attributes|
| `0x1C` |DWORD|Extended Size|
| `0x20` |WORD|Configured Memory Speed|
| `0x22` |WORD|Minimum Voltage|
| `0x24` |WORD|Maximum Voltage|
| `0x26` |WORD|Configured Voltage|
| `0x28` |BYTE|Memory Technology|
| `0x29` |WORD|Memory Operating Mode Capability|
| `0x2B` |STRING|Firmware Version|
| `0x2C` |WORD|Module Manufacturer ID|
| `0x2E` |WORD|Module Product ID|
| `0x30` |WORD|Memory Subsystem Controller Manufacturer ID|
| `0x32` |WORD|Memory Subsystem Controller Product ID|
| `0x34` |QWORD|Non-Volatile Size|
| `0x3C` |QWORD|Volatile Size|
| `0x44` |QWORD|Cache Size|
| `0x4C` |QWORD|Logical Size|
| `0x54` |DWORD|Extended Speed|
| `0x58` |DWORD|Extended Configured Memory Speed|

完整 Type 17 的格式化区域长度为 `0x5C`。

Type 17 的 `Physical Memory Array Handle` 用于关联 Type 16；同一 Type 16 可被多个 Type 17 引用。

### 4.8 Type 32：System Boot Information

Type 32 描述最近一次系统启动的状态。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `32` |
| `0x01` |BYTE|Length|
| `0x02` |WORD|Handle|
| `0x04` |6 BYTE|Reserved|
| `0x0A` |BYTE|Boot Status|

完整 Type 32 的格式化区域长度为 `0x0B`。

### 4.9 Type 127：End-of-Table

Type 127 是 SMBIOS 表结束标记。

|偏移|长度|字段|
|---|---|---|
| `0x00` |BYTE|Type，固定为 `127` |
| `0x01` |BYTE|Length，通常为 `4` |
| `0x02` |WORD|Handle|

解析程序遇到 Type 127 后应停止继续解析结构表。

## 5. 二进制格式解析

以下为一个简化的 Type 1 示例：

```
01 08 01 00 01 02 03 04
41 43 4D 45 00
53 65 72 76 65 72 20 58 31 00
31 2E 30 00
53 4E 31 32 33 34 35 36 00
00
```

前 8 字节为格式化区域：

|字节|解释|
|---|---|
| `01` |Type 1：System Information|
| `08` |格式化区域长度为 8 字节|
| `01 00` |Handle 为 `0x0001` |
| `01` |Manufacturer，引用字符串 1|
| `02` |Product Name，引用字符串 2|
| `03` |Version，引用字符串 3|
| `04` |Serial Number，引用字符串 4|

字符串区域解析结果：

```
Manufacturer: ACME
Product Name: Server X1
Version: 1.0
Serial Number: SN123456
```

## 6. Linux 中获取 SMBIOS 信息

使用 `dmidecode` 查看解析后的信息：

```
sudo dmidecode
sudo dmidecode -t bios
sudo dmidecode -t system
sudo dmidecode -t processor
sudo dmidecode -t memory
```

Linux 内核通常将原始 SMBIOS 数据导出为：

```
/sys/firmware/dmi/tables/smbios_entry_point
/sys/firmware/dmi/tables/DMI
```

可查看原始字节：

```
sudo xxd /sys/firmware/dmi/tables/smbios_entry_point
sudo xxd /sys/firmware/dmi/tables/DMI
```

## 7. SMBIOS 分层协议

#信息来源 ： BIOS

SMBIOS 标准定义的是数据格式和入口点，不规定“主机必须如何把 SMBIOS 数据传给 BMC”。因此，物理通道和传输协议取决于具体服务器平台。

下图为 OpenBMC 中常见的实现路径：

```
BIOS / UEFI
    │  生成 SMBIOS Entry Point + Structure Table
    ▼
主机内存或主机侧暂存区
    │
    │  MDRv2 / IPMI Blob 等平台机制
    ▼
BMC 侧缓存文件或共享内存
    │
    ▼
smbios-mdr
    │  解析 Type 0、1、2、3、4、16、17 等
    ▼
D-Bus Inventory / SMBIOS 对象
    │
    ├── bmcweb / Redfish
    ├── IPMI 服务
    ├── Web UI
    └── 其他 BMC 资产管理服务
```


+ 信息类型：主要提供静态或启动时确定的资产信息，如：CPU、Memory、BIOS、GPU、PCIE devices etc；
+ 数据： BIOS/UEFI   → 生成 SMBIOS 二进制表，在主机内存中；
+ 物理链路：
    + MDRV 2 路径：通过 PCIe Memory Write 向 BMC 暴露的 VGA BAR 中约定的共享内存区域写入 SMBIOS 表，BMC 从本地 DRAM 的同一片区域读取
    + IPMI Blob 路径：ipmi over kcs 进行传输，物理链路是 LPC 或 eSPI 总线；
+ 传输层：
    + **MDRv2 路径**：BIOS 通过 Intel OEM IPMI 的 MDRv2 协议通知并传输数据；BMC 侧 `intel-ipmi-oem` 处理命令。
    + **IPMI Blob 路径**：BIOS 通过 IPMI Blob API 分块上传 `/smbios`；BMC 上的 Blob handler 将数据写到 `/var/lib/smbios/smbios2`，然后调用 `AgentSynchronizeData` 通知 `smbios-mdr` 重载和解
+ 应用层：SMBios 标准
+ 常见 openbmc 服务：`smbios-mdr` 是常见的 SMBIOS 解析服务，`ipmi-blobs` 提供 ipmi blob 数据传输能力，ipmid 接收主机侧的 IPMI 或 OEM IPMI 数据；

调试主要路径：

```
BIOS 是否生成
    ↓
主机是否发送
    ↓
BMC 是否接收并保存原始表
    ↓
smbios-mdr 是否成功解析
    ↓
D-Bus 对象是否出现
    ↓
Redfish / Web UI 是否正确映射
```

