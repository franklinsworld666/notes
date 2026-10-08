# OpenBMC SNMP 功能设计与实现

## 1. SNMP 协议基础

### 1.1 管理模型：管理端、Agent、MIB 与 OID

外部网管系统是 SNMP 管理端，BMC 上的 `snmpd` 是 Agent。管理端发送查询请求，Agent 根据请求中的对象标识符（OID）定位处理模块，读取 BMC 内部数据并返回结果。

MIB 定义对象的名称、OID、类型、访问权限与语义；它帮助双方按同一套规则解释数据，但 MIB 文件本身不负责读取硬件。

例如，某个温度传感器在 OpenBMC 内部表现为 D-Bus 对象，其 `Value` 属性由传感器服务更新；`libsnmp_mib.so` 将该对象映射为 MIB 表中的一行。外部管理端查询该行的 OID 时，得到的是转换后的 SNMP 值。

MIB 中通常使用两类对象：

| 类型 | OID 形式 | 适用数据 |
| --- | --- | --- |
| 标量 | 对象 OID 后加实例 `.0` | BMC 名称、固件版本、主机状态等单个值 |
| 表 | 表 OID / entry / 列号 / 行索引 | 传感器、风扇、电源等数量可变的数据 |

私有 MIB 必须使用项目拥有或获授权使用的企业编号。发布 MIB 时，应同时发布列号、索引规则、单位和状态枚举。

### 1.2 查询操作与 `snmpwalk`

| 操作 | 含义 | 在本项目中的用途 |
| --- | --- | --- |
| GET | 读取精确 OID | 查询一个标量或表的某个单元格 |
| GETNEXT | 读取字典序上紧随请求 OID 的可访问对象 | 遍历下一行或下一列 |
| GETBULK | 批量执行后继查找，减少往返（v2c/v3） | 大表快速遍历 |
| SET | 修改对象 | 本文按只读设计，不开放写入 |

`snmpwalk` 是客户端工具，连续发出 GETNEXT 来遍历子树；

使用 GETBULK 遍历时对应的工具是 `snmpbulkwalk`。



### 1.3 Trap 与 Inform

Trap 是 BMC 主动发出的通知。告警服务在错误日志或监控事件出现后构造通知 OID 和变量绑定，向配置的管理端发送报文。Trap 本身没有协议级确认；管理端未收到时，发送端不能仅凭 `snmp_send()` 成功就认定告警已送达。Inform 则要求接收方响应，并可由发送方处理超时或重试。

### 1.4 版本与安全

SNMPv2c 用 community 字符串标识访问者，报文本身没有加密保护。

SNMPv3 的 USM 提供 `noAuthNoPriv`、`authNoPriv`、`authPriv` 等安全级别；其中认证校验报文来源与完整性，隐私保护对报文加密。鉴权后，VACM 再决定用户能访问哪些 OID。 



SNMPv1、v2c、v3 的主要差异如下。v3 沿用 v2 的主要管理操作，并增加消息安全机制；实际能否使用某个算法和对象类型，还取决于 Agent 的编译选项与 MIB 实现。

| 维度 | SNMPv1 | SNMPv2c | SNMPv3（常用 USM） |
| --- | --- | --- | --- |
| 身份识别 | community 明文随报文发送 | community 明文随报文发送 | 用户名与 EngineID；启用认证时还使用密钥 |
| 安全能力 | 无报文加密、完整性校验 | 与 v1 相同 | `noAuthNoPriv`、`authNoPriv`、`authPriv`；VACM 控制 OID 访问 |
| 查询操作 | GET、GETNEXT、SET | 增加 GETBULK | 支持 GETBULK 等 v2 操作 |
| 通知 | v1 Trap | v2 Trap、Inform（有响应） | v2 格式的 Trap、Inform，并可应用 v3 安全机制 |
| 数据和错误 | 不支持 Counter64；错误通常针对整个请求 | 支持 Counter64、逐变量异常（如 `noSuchInstance`） | 与 v2 的主要对象类型和异常语义一致 |

因此，本项目若仅做只读查询，v2c 与 v3 的 MIB 数据映射可以共用；生产环境优先启用 v3 `authPriv`。v1 仅在确有旧网管兼容要求时启用。v1/v2c 的 community 不是 SNMPv3 用户，也不提供加密。[RFC 3410](https://www.rfc-editor.org/rfc/rfc3410.html)、[RFC 3584](https://www.rfc-editor.org/rfc/rfc3584.html)、[RFC 3414](https://www.rfc-editor.org/rfc/rfc3414.html)

### 1.5 查询命令

```shell
snmpget  -v2c -c '<community>' <BMC_IP> '<一个标量OID>.0'
snmpwalk -v2c -c '<community>' <BMC_IP> '<传感器表OID>'
snmpget   -v3 -l authPriv -u '<用户>' -a '<认证算法>' -A '<认证口令>' \
  -x '<加密算法>' -X '<加密口令>' <BMC_IP> '<一个标量OID>.0'
```



### 1.6 网络路径

管理端查询：`管理端 → BMC:161/UDP → snmpd → 响应返回管理端`。

Trap：`BMC → 管理端:162/UDP`，实际接收端口可配置。需要核对 BMC 监听地址、路由、防火墙和管理网隔离策略；打开 161 端口只服务于查询，发送 Trap 不要求 BMC 在 162 端口监听。



## 2. 总体架构与数据流

```text
                         GET / GETNEXT / GETBULK
外部管理端 ───────────────────────────────→ snmpd:161
    ↑                                         │
    │ RESPONSE                                │ 调用 MIB 模块
    └─────────────────────────────────────────┤
                                              ↓
                                       libsnmp_mib.so
                                       表/标量处理器、缓存
                                              │ 读取
                                              ↓
                                       OpenBMC D-Bus 服务
                                       传感器、状态、资产等

OpenBMC 事件/日志 → 触发规则/监控服务 → phosphor-snmp/libsnmp
                                              │ Trap
                                              ↓
                                      外部管理端:162
```

图中的 `libsnmp_mib.so` 是你提供的项目实现名称。它与 `snmpd` 的连接方式可能是编译时链接、动态加载或其他注册机制；本文只描述 MIB 处理器被注册并由 Agent 调用后的逻辑，不假定具体加载命令。

### 2.1 查询链路

1. 管理端发送带 OID 的请求。`snmpd` 完成报文解析、版本处理、身份验证与访问控制。
2. Agent 在已注册的 OID 子树中找到 `libsnmp_mib.so` 提供的处理器。
3. 对表请求，Net-SNMP 的 helper 解析列和索引，并通过缓存及迭代器定位行。
4. 项目处理函数从缓存行中取值，按 MIB 定义完成数值范围、单位和 ASN.1 类型转换，写入响应变量绑定。
5. Net-SNMP 组装 RESPONSE 并发回管理端。

### 2.2 Trap 链路

1. OpenBMC 的日志、传感器或设备状态服务产生事件。
2. 监控规则识别需上报的事件，取得事件 ID、时间、级别、消息和设备信息。
3. 发送库根据 MIB 中的通知定义，加入 `sysUpTime.0`、`snmpTrapOID.0` 及业务字段。
4. 发送库读取目标管理端配置，逐个创建 SNMP 会话并发送 Trap。上游文档将 `phosphor-dbus-monitor` 与 `phosphor-snmp` 作为这一链路的组成部分。[OpenBMC SNMP 配置说明](https://github.com/openbmc/phosphor-snmp/blob/master/docs/snmp-configuration.md)

### 2.3 查询值与告警的一致性

查询接口展示当前状态，Trap 记录状态变化或事件。两者应使用一致的设备标识、传感器名称、单位和严重级别定义。Trap 发生后，管理端可再次查询对应 OID 确认当前状态；由于查询缓存可能仍在 5 秒有效期内，短时间内看到旧值属于需要明确设计的窗口。可在关键事件后主动使缓存失效，或在产品说明中标注最坏数据陈旧时间。

## 3. `snmpd` 查询功能

### 3.1 查询对象清单

下表是建议的 MIB 规划清单。只有完成 OID 分配、数据映射和验证的对象才能标为“已支持”。

| 对象组 | 代表字段 | 建议数据来源 | 形式 |
| --- | --- | --- | --- |
| BMC 基本信息 | 名称、运行时间、固件版本 | 系统配置、软件管理服务 | 标量 |
| 设备资产 | 型号、序列号、在位状态 | Inventory 相关 D-Bus 对象 | 表 |
| 传感器 | 名称、类型、当前值、阈值、可用状态 | `xyz.openbmc_project.Sensor.*` | 表 |
| 主机与机箱 | 电源状态、运行状态 | State 相关 D-Bus 对象 | 标量或表 |

### 3.2 MIB 与数据映射

每个对象应在映射表中记录：**符号名、数字 OID、类型、单位、取值范围、只读权限、D-Bus 服务/对象/接口/属性、缺失时的行为、缓存时长**。例如，D-Bus 的传感器值可能是浮点数，而 MIB 列定义为整数；这时必须写明倍率（如乘以 1000）、舍入方式和越界处理，不能把 C++ 内存中的浮点字节直接按 ASN_INTEGER 回传。

表索引必须在同一台设备上尽量稳定。若仅按 D-Bus 枚举顺序分配 1、2、3，重启或热插拔后顺序变化，管理端可能把旧传感器误认为新传感器。建议建立稳定设备键与行索引的映射，并在 MIB 中说明索引重用规则。



### 3.3 MIB 文件示例：一个标量和一个表

下面的 SMIv2 示例定义 BMC 名称标量和传感器表。`999999` 仅作演示，发布前必须替换为项目拥有或获授权使用的企业编号；名称、量程和单位也应按产品实际定义。MIB 描述数据结构，`libsnmp_mib.so` 等处理器才负责从 D-Bus 取值。

```asn1
BMC-DEMO-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE, Integer32, enterprises
        FROM SNMPv2-SMI;

bmcDemoMib MODULE-IDENTITY
    LAST-UPDATED "202610080000Z"
    ORGANIZATION "Example only"
    CONTACT-INFO "Replace with project contact"
    DESCRIPTION "Illustrative, read-only OpenBMC objects."
    REVISION "202610080000Z"
    DESCRIPTION "Initial example."
    ::= { enterprises 999999 }

bmcName OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (0..64))
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION "The BMC name."
    ::= { bmcDemoMib 1 }

sensorTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF SensorEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION "A table of sensor readings."
    ::= { bmcDemoMib 2 }

sensorEntry OBJECT-TYPE
    SYNTAX      SensorEntry
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION "One sensor row."
    INDEX       { sensorIndex }
    ::= { sensorTable 1 }

SensorEntry ::= SEQUENCE {
    sensorIndex       Integer32,
    sensorName        OCTET STRING,
    sensorMilliCelsius Integer32
}

sensorIndex OBJECT-TYPE
    SYNTAX      Integer32 (1..2147483647)
    MAX-ACCESS  not-accessible
    STATUS      current
    DESCRIPTION "Stable index assigned to this sensor."
    ::= { sensorEntry 1 }

sensorName OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (0..64))
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION "The sensor name."
    ::= { sensorEntry 2 }

sensorMilliCelsius OBJECT-TYPE
    SYNTAX      Integer32 (-40000..125000)
    UNITS       "millidegrees Celsius"
    MAX-ACCESS  read-only
    STATUS      current
    DESCRIPTION "Current temperature, scaled by 1000."
    ::= { sensorEntry 3 }

END
```

替换企业编号后，`bmcName.0` 是标量实例（示例 OID `.1.3.6.1.4.1.999999.1.0`）；索引为 1 的 `sensorMilliCelsius` 是表单元格（`.1.3.6.1.4.1.999999.2.1.3.1`）。`sensorTable` 和 `sensorEntry` 本身不能直接 GET；表列的末尾索引来自 `INDEX { sensorIndex }`。



### 3.4 `libsnmp_mib.so` 的注册与回调

以下使用你给出的 `mySensorTable_handler`、`sensor_cache_load` 名称描述典型的 Net-SNMP `table_iterator + cache` 设计。实际注册代码应确认 helper 的注入顺序。

| 函数或回调 | 作用 | 返回或结果 |
| --- | --- | --- |
| `init_mySensorTable()` | 注册表 OID、索引类型、最小/最大列；设置 iterator 和 cache handler | 注册成功后该子树可被 `snmpd` 找到 |
| `sensor_cache_load(netsnmp_cache *, void *)` | 读取 D-Bus，生成当前表快照 | **0 或正数为成功，负数为加载失败**；成功但零行也返回成功 |
| `sensor_cache_free(netsnmp_cache *, void *)` | 释放旧快照 | 不返回值；不能释放仍被请求使用的行数据 |
| `get_first_data_point(...)` | 给出第一行索引、遍历上下文和数据上下文 | 有行时返回索引变量绑定；空表返回 `NULL` |
| `get_next_data_point(...)` | 移到下一行并更新上下文 | 有下一行时返回索引变量绑定；表尾返回 `NULL` |
| `mySensorTable_handler(...)` | 根据列号和行上下文填充结果 | 为每个有效请求变量绑定赋值；函数返回值按 Net-SNMP handler 约定处理 |

缓存 helper 按超时时间检查是否需要加载。这里的 **5 秒不是后台每 5 秒自动采集**：默认情况下，首次请求或缓存过期后的下一次请求触发 `sensor_cache_load`；仍在有效期内的请求直接使用快照。只有显式启用自动重载等选项时，行为才可能变为定时加载。Net-SNMP 文档规定加载回调以负值表示失败，以 0 或正值表示成功。[Net-SNMP 缓存 API](https://net-snmp.sourceforge.io/docs/man/netsnmp_cache_handler.html)

### 3.5 一次传感器表请求的调用顺序

假设请求目标为“列 2、索引 1”，且项目把 cache 注入在 iterator 之前，典型流程为：

```text
请求进入 snmpd
  → 版本、USM/community、VACM 检查
  → 注册子树的 handler 链
  → cache helper：检查 5 秒有效期，必要时调用 sensor_cache_load
  → table / table_iterator helper：解析列与索引，检查列范围
  → get_first_data_point：提供第一行
  → get_next_data_point：按需继续遍历，寻找 GET 的精确行或 GETNEXT 的后继行
  → mySensorTable_handler(MODE_GET)：读取被选中行的列值
  → snmp_set_var_typed_value：填写该请求的 varbind
  → Net-SNMP 组装并发送 RESPONSE
```

**GET 与 GETNEXT 的细节不同。** GET 必须找到精确实例。GETNEXT 寻找严格大于请求 OID 的字典序后继。GETBULK 由 Agent 转换为多次后继查找，可能重复触发迭代器回调，不能假设整个请求只调用一次 `get_first_data_point`。[Net-SNMP table iterator](https://net-snmp.sourceforge.io/dev/agent/group__table__iterator.html)、[iterator 源码](https://github.com/net-snmp/net-snmp/blob/master/agent/helpers/table_iterator.c)

上图是**逻辑顺序**。在实际 Net-SNMP 版本和注册方式下，`table`、`cache`、`iterator` 的物理排列由 handler 链决定，不能仅凭函数名固定断言为“table → cache → iterator”。建议在目标固件中打开相应调试日志，核对最终顺序。

### 3.6 无数据与错误如何返回

必须区分“确实没有对象”和“读取对象时失败”。空表不应伪造一行全 0 的数据，也不应把 `sensor_cache_load` 返回负数。

| 场景 | 回调处理 | 管理端应观察到的语义 |
| --- | --- | --- |
| D-Bus 读取成功，表为空 | `sensor_cache_load` 返回 0；`get_first_data_point` 返回 `NULL` | 精确 GET 没有该实例；遍历跳过该表或到达视图末尾 |
| 已到最后一行 | `get_next_data_point` 返回 `NULL` | GETNEXT/GETBULK 查找下一列、下一个子树，或最终得到 `endOfMibView` |
| 表存在，请求索引不存在 | iterator 不交付匹配的数据上下文；业务 handler 不写入虚构值 | 精确 GET 通常表现为 `noSuchInstance` |
| 请求了未定义列或 OID | 由 table helper 或处理器拒绝 | `noSuchObject` / `noSuchInstance`，取决于 OID 是否落在已定义对象前缀内 |
| D-Bus 暂时失败 | `sensor_cache_load` 返回负值，并记录日志；可按项目策略保留旧快照或报告处理错误 | 不应把“采集失败”误报为“设备不存在” |

如果某个有效行的**某一列**无值，需要先在 MIB 中决定该列是否允许稀疏。如果允许稀疏，GET 的缺失单元格可通过 Net-SNMP 请求异常 API 表达 `SNMP_NOSUCHINSTANCE`；GETNEXT 应继续寻找后继列或行。如果该列必须存在，则应把缺值视作采集异常并记录，而非返回任意整数。

`netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE)` 与 `snmp_set_var_typed_value()` 的用途不同：前者设置异常，后者写入正常值。具体异常转发仍需在目标版本的 helper 链上验证。[RFC 3416](https://www.rfc-editor.org/rfc/rfc3416.html)、[Net-SNMP table helper 源码](https://github.com/net-snmp/net-snmp/blob/master/agent/helpers/table.c)

### 3.7 性能与一致性

5 秒缓存降低每个 varbind 都访问 D-Bus 的成本，但也允许最多约 5 秒的数据陈旧窗口；准确窗口还取决于源 D-Bus 属性自身的更新频率。缓存加载应先构建完整新快照，再原子地替换旧快照，以免 `get_first_data_point` 与业务 handler 看到不同的数据。每次迭代通常都可能扫描多行，大表执行 `snmpwalk` 时应记录总时延和 CPU 使用率；若性能不足，可评估已排序索引、其他表 helper 或更长缓存时间。[Net-SNMP table iterator 说明](https://net-snmp.sourceforge.io/wiki/index.php/Table_iterator)



## 4. 配置

### 4.1 `snmpd` 查询配置

配置要覆盖服务启用开关、监听地址、协议版本、访问控制、可见 OID 范围以及 SNMPv3 用户。

配置片段示意如下，`<...>` 必须替换为项目值，不能直接部署：

```text
# 仅当固件支持 dlmod，且动态库导出 init_mySensorTable() 时使用
dlmod mySensorTable /usr/lib/libsnmp_mib.so

# 只允许管理网访问；具体监听地址由平台填写
agentAddress udp:<BMC_管理网地址>:161

# 若使用 AgentX 子代理才需要；内置 MIB 模块不依赖此开关
# master agentx

# v2c 兼容配置：限制来源与可见子树
rocommunity <随机且独立的community> <管理端地址或网段> <允许的OID子树>

# v3 授权：用户须另行创建，priv 表示要求认证和加密
rouser <只读用户名> priv <允许的OID子树>

# 各协议版本是否启用
[snmp] disableSNMPv1 no
[snmp] disableSNMPv2c no
[snmp] disableSNMPv3 no
```



### 4.2 v2c 与 v3 用户、密钥的添加和保存

先区分三个概念：

**v2c 的 community** 是报文中携带的共享字符串，并不存在“v2c 用户”；

**v3 的 USM 用户**保存用户名、认证/加密算法及密钥；

**VACM 授权规则** 再决定该身份能访问哪些 OID。

| 内容 | 常见配置位置 | 作用 |
| --- | --- | --- |
| v1/v2c community 及其来源、OID 限制 | 常规 `snmpd.conf`，常见如 `/etc/snmp/snmpd.conf` | `rocommunity` 或 `com2sec` 等规则把 community 映射为可授权的身份 |
| v3 USM 用户、EngineID、密钥材料 | `snmpd` 的持久化文件，默认 `/var/net-snmp/snmpd.conf` | 重启后仍能识别该用户并校验/解密报文 |
| v3 用户的 OID 访问权限 | 常规 `snmpd.conf` | `rouser` 简化规则，或 4.3 节的 `group`、`view`、`access` 完整 VACM 规则 |

**添加一个只读 `authPriv` 用户的典型步骤：**

1. 停止 `snmpd` 后，在**持久化文件**中增加一行（以下口令均为占位符，不要把真实口令提交到镜像或版本库）：

   ```text
   createUser bmcReader SHA "<认证口令>" AES "<加密口令>"
   ```

   认证和加密口令应不同，均满足目标 Net-SNMP 版本的长度要求；`SHA`、`AES` 的可用性以固件编译结果为准。若镜像带用户创建脚本，也可在停止 Agent 后使用 `net-snmp-config --create-snmpv3-user` 或 `net-snmp-create-v3-user`；按该版本的帮助选择只读、SHA 和 AES，并检查脚本实际写入的文件与授权规则。部分脚本会把含明文口令的配置行回显到终端，操作日志须妥善保护。[Net-SNMP 创建脚本](https://github.com/net-snmp/net-snmp/blob/master/net-snmp-create-v3-user.in)
3. 在**常规 `snmpd.conf`** 中授予只读权限，例如 `rouser bmcReader priv .1.3.6.1.2.1.1`；若采用 4.3 节的完整 VACM 示例，则用其中的 `group`、`view`、`access` 规则授权，不再叠加这条 `rouser`。`priv` 表示请求至少达到 `authPriv` 安全级别。
4. 启动 `snmpd`。Agent 读取 `createUser` 后，先由口令派生密钥，再结合本机 EngineID 生成**本地化密钥**；用户通常以 `usmUser` 等记录保存在持久化 `snmpd.conf` 中，而明文 `createUser` 行会被移除。检查实际持久化文件中的用户记录，并用 `authPriv` 查询一个允许的 OID 验证授权；若 Agent 要在正常停止时才写回持久化状态，应在安全停机后复查。不要把密钥记录当作可移植的用户配置复制到另一台 BMC；EngineID 改变也可能使原记录失效。[Net-SNMP v3 用户说明](https://www.net-snmp.org/docs/man/snmpd.conf.html)、[RFC 3414](https://www.rfc-editor.org/rfc/rfc3414.html)



### 4.3 VACM 访问控制的总体框架

VACM（View-based Access Control Model，基于视图的访问控制）解决的是**已识别的请求可以访问哪些对象**。它本身不验证 v3 口令，也不读取传感器。`snmpd` 先完成 v2c community 映射或 v3 USM 认证，再把请求的身份和安全属性交给 VACM。VACM 为每个请求中的 OID 判断是否可读或可写，只有通过授权的对象才进入相应 MIB 处理流程。[RFC 3415](https://www.rfc-editor.org/rfc/rfc3415.html)

```text
收到 SNMP 请求
  → v2c：按来源地址 + community 映射出 securityName
    v3：USM 验证用户名、认证/加密，得到 securityName
  → (securityModel, securityName) 查找 group
  → (group, context, securityModel, securityLevel) 查找 access
  → 按操作选 READ 或 WRITE 视图
  → 判断目标 OID 是否属于该 view
  → 允许：继续调用 MIB handler；拒绝：返回访问失败/不可见的语义
```

| VACM 概念 | 作用 | 本文示例 |
| --- | --- | --- |
| `securityModel` | 身份来自哪套安全模型 | `v2c` 或 `usm`（v3 常用） |
| `securityName` | 模型交给 VACM 的身份名 | v2c 由 `com2sec` 映射成 `bmcV2`；v3 为已认证的 `bmcReader` |
| `securityLevel` | 请求实际采用的安全级别；`access` 的 `LEVEL` 规定最低要求 | v2c 用 `noauth`；v3 的 `priv` 要求 `authPriv` |
| `context` | 一组独立的管理对象空间 | `""` 表示默认 context；普通单实例 Agent 通常只用它 |
| `group` | 把身份归入授权角色 | `bmcV3Readers` |
| `view` | 由 `included` / `excluded` OID 子树构成的可见集合 | 只包含 `system` 子树 |
| `access` | 按组、context、模型和级别，选择读/写/通知视图 | 只给 READ 视图，WRITE 为 `none` |

Net-SNMP 用四类配置表达这套关系：

`com2sec` 把 v1/v2c 的来源地址和 community 映射为安全名；v3 已由 USM 提供安全名，不经过 `com2sec`。

`group` 把安全模型与安全名映射到组；

`access` 把组及请求条件映射到读、写、通知视图。

`view` 定义可见 OID；

`rocommunity` / `rouser` 是适合简单场景的快捷写法，本质上仍会形成访问控制规则。[Net-SNMP VACM 配置](https://www.net-snmp.org/docs/man/snmpd.conf.html)

下面的完整示例允许指定管理端使用 v2c，或让已按 4.2 节创建的 `bmcReader` 使用 v3 `authPriv`，**只读** `system` 子树。占位符必须替换，且这组规则应与 4.1 节的快捷 `rocommunity` / `rouser` 示例择一使用。

```text
# v2c：来源地址 + community → 安全名；v3 不需要此行
com2sec bmcV2 192.0.2.10/32 <独立community>

# (安全模型, 安全名) → 组
group bmcV2Readers v2c bmcV2
group bmcV3Readers usm bmcReader

# 可读对象集合：system 子树
view bmcSystemView included .1.3.6.1.2.1.1

# access GROUP CONTEXT MODEL LEVEL PREFIX READ WRITE NOTIFY
access bmcV2Readers "" v2c noauth exact bmcSystemView none none
access bmcV3Readers "" usm priv exact bmcSystemView none none
```

以最后一行为例：`bmcV3Readers` 是组；`""` 是默认 context；`usm` 指 v3 用户安全模型；`priv` 要求 `authPriv`；`exact` 表示 context 名必须精确匹配；后三列依次为 READ、WRITE、NOTIFY 视图。`bmcSystemView` 允许读取该视图中的 OID，`none` 表示没有对应视图，因此不授权写入。NOTIFY 列是 VACM 的通知视图字段；在 Net-SNMP 的这类配置中，不应将它理解为主动发送 Trap 的开关。[Net-SNMP `snmpd.conf`](https://www.net-snmp.org/docs/man/snmpd.conf.html)

例如，`bmcReader` 用 `authPriv` GET `sysDescr.0`（`.1.3.6.1.2.1.1.1.0`）：USM 验证通过，`group` 找到 `bmcV3Readers`，`access` 选中 `bmcSystemView`，目标 OID 属于 `system`，因此可读取。若同一用户改用 `authNoPriv`，就达不到 `priv` 要求；若查询 `ifDescr`（不在该视图内），即使身份验证通过也无权读取；SET 请求因 WRITE 为 `none` 被拒绝。v2c 请求若来源不是 `192.0.2.10/32`，则在 `com2sec` 阶段就匹配不到该身份。要开放项目私有 MIB，可再加入其企业 OID 子树的 `view ... included ...` 规则，并核对可见范围。[RFC 3415](https://www.rfc-editor.org/rfc/rfc3415.html)



## 5. 安全设计

### 5.1 默认只读

MIB 查询对象注册为只读，只实现 GET 相关处理；`snmpd.conf` 使用 `rocommunity` / `rouser` 或等价 VACM 视图，不授予 SET 权限。即使将来需要远程控制 BMC，也应单独评审可写对象和审计要求，不能把查询表直接改为读写。

### 5.2 v2c 如何鉴权与授权

v2c 报文携带 community 字符串。`snmpd` 将它与配置规则匹配，并可同时限制请求来源和可访问的 OID 子树。匹配失败时不会把请求交给业务 MIB handler。community 在网络上传输时不提供加密，因此应只在受控管理网络使用，避免默认或共享的 `public`，并限制监听地址与访问来源。

### 5.3 v3 如何鉴权，用户数据在哪里

SNMPv3 USM 按用户名、EngineID 和密钥材料进行消息级处理；`authPriv` 同时要求认证和加密。通过 USM 后，VACM 的 `rouser` 或视图规则决定可读取的 OID。

USM 用户和本地化密钥保存在 `snmpd` 的持久化配置中，OID 授权规则保存在常规配置或平台生成的授权配置中。添加用户、确定实际文件位置及验证跨重启保存的方法见 4.2 节；VACM 如何决定可访问的 OID 见 4.3 节。