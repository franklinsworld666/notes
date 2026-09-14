# 1. FRU 是什么，是否有格式标准

## 1.1 概念

**FRU = Field Replaceable Unit（现场可更换单元）**。在服务器/计算节点里，凡是能"拔下来换一个"的部件（主板、电源、风扇、背板、CPU、内存、网卡、HBA、硬盘背板等）都属于 FRU。

为了让 BMC 在不依赖 OS 的情况下也能知道"这台机器上插了什么、序列号是多少、厂商是谁、部件号是多少"，IPMI 规范规定：每个 FRU 都要带一片小 EEPROM（类型有： 24C02 / 24C04/24C32, 容量大小不同），里面按统一格式存储该部件的"身份信息"。这片 EEPROM 就叫 **FRU EEPROM**，里面的内容就叫 **FRU 数据**。

## 1.2 格式标准

有，而且是非常严格的二进制标准：

- **规范文档**：Intel 发布的《**IPMI Platform Management FRU Information Storage Definition v1.3**》。
- 它是 IPMI 规范族的一部分，与《IPMI v2.0 Specification》配套使用。
- 规范把整片 EEPROM 划分成若干个 **Area（区）**，每个 Area 可以存在也可以不存在，由 Common Header 中的偏移指针决定。

## 1.3 整体布局

![FRU整体布局](fru-笔记-20260914084445824.png)

## 1.4 Common Header

![Common Header结构](fru-笔记-20260914084445825.png)

**checksum 计算** : 
```python
sum( data[0:9]) & 0xFF = 0

```

## 1.5 internal 内部结构

厂商自定义

## 1.6 Chassis Area 内部结构

![Chassis Area内部结构](fru-笔记-20260914084445825-1.png)


**Type/Length Byte** 是 FRU 格式的精髓，1 字节同时表示类型和长度：


![Type/Length Byte说明](fru-笔记-20260914084445826.png)

例如 0xC8 = 11 001000 → 8-bit ASCII，长度 8。

如有有信息只有1字节，刚好可能出错。

## 1.7 Board Area 内部结构

![Board Area内部结构](fru-笔记-20260914084445826-1.png)

- date ：
- Manufacture
- Product Name
- Serial Number
- Part Number/Model
- FRU File ID
- custom info

## 1.8 Product Info Area 内部结构

![Product Info Area内部结构](fru-笔记-20260914084445827.png)

Product Info Area 包含：

- Manufacturer
- Product Name
- Part/Model Number
- Version
- Serial Number
- Asset Tag
- FRU File ID
- Custom fields

## 1.9 MultiRecord Area

用于标准没覆盖的扩展信息（如 PSU 的电压电流、DC Load、MAC 地址、PCIe slot 映射等）。

每条 record 有：

![Record结构](fru-笔记-20260914084445827-1.png)

record checksum: record 数据部分checksum；

header checksum: 前4字节 checksum;

Record Type ID 范围：

- 0x00 ~ 0xBF：标准化

![Record Type ID范围](fru-笔记-20260914084445827-2.png)

- 0xC0 ~ 0xFF：OEM 自定义，从0xC0 开始

在multirecord中，每一个 record type 允许多条，这就造成读取是ok的，但是修改是复杂的。

# 2. 读写 fru

```
# 把本地 fru.bin 写到 BMC 的 FRU id=0
# fru 0 通常就是主板上的 eeprom
ipmitool -I lanplus -H <bmc_ip> -U <user> -P <pass> fru write 0 fru.bin

# 显示
ipmitool fru list

# 读取
ipmitool fru read 0 fru.bin

# 修改某些字段
```

![修改字段示例](fru-笔记-20260914084445828.png)


## Eeprom 设备 

```bash
# 创建设备节点
echo 24c32 addr > /sys/bus/i2c/devices/i2c-bus/new_device

echo addr > /sys/bus/i2c/devices/i2c-bus/delete_device
```


# 3. ipmi 如何获取到FRU

ipmitool 并不直接读 I2C，它走的是 **IPMI Storage 命令（NetFn=0x0A）**，3 条核心命令：（从dbus读取的）

| 命令 | NetFn/Cmd | 作用 |
|------|-----------|------|
| Get FRU Inventory Area Info | 0x0A / 0x10 | 返回 FRU 总大小 |
| Read FRU Data | 0x0A / 0x11 | 按偏移读一段字节 |
| Write FRU Data | 0x0A / 0x12 | 按偏移写一段字节 |

相关代码：

- phosphor-ipmi-host/storagehandler.cpp
- ipmiStorageGetFruInvAreaInfo
- ipmiStorageReadFruData
