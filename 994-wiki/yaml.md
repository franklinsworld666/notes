# YAML 精简教程 —— 以 FRU 文件为例

## 一、什么是 YAML

YAML（YAML Ain't Markup Language）是一种**人类可读的数据序列化格式**，广泛用于配置文件、数据交换和基础设施描述。它的核心设计理念是：

- 用**缩进**表示层级关系
- 用**键值对**组织数据
- 语法简洁，可读性强

---

## 二、基本语法规则

| 规则 | 说明 |
|------|------|
| 缩进 | 使用**空格**缩进，禁止使用 Tab。通常 2 个空格为一级 |
| 换行 | 用 `\n` 换行 |
| 大小写 | **大小写敏感**，`Name` 和 `name` 是不同的键 |
| 注释 | 用 `#` 开头，从 `#` 到行尾的内容会被忽略 |
| 冒号 | 键值对用 `:` 分隔，冒号后**必须**跟一个空格或换行 |

---

## 三、键值对（Mappings）

键值对是 YAML 最基础的元素，大小写敏感，格式为 `键: 值`。

**FRU 文件示例：**

```yaml
BoardInfo:
  manufacturer: Aquarius
  part_number: AQC621AC-MB-12-01
  product_name: AQC621AC
  serial_number: AQ224070119
```

解读：
- `BoardInfo` 是一个顶层键，它的值是一个嵌套的映射（子键值对）
- 缩进 2 个空格表示 `manufacturer`、`part_number` 等属于 `BoardInfo` 的子项
- 冒号后面紧跟一个空格，然后是值

---

## 四、字符串与引号

YAML 中字符串**可以不加引号**，但在以下情况建议加引号：

| 场景 | 示例 | 说明 |
|------|------|------|
| 空字符串 | `serial_number: ''` | 不加引号会被解析为 `null` |
| 纯数字字符串 | `type: '23'` | 不加引号会被解析为整数 |
| 含特殊字符 | `part_number: 'T50 D224AC'` | 防止歧义 |
| 字符串开头空格 | `part_number: '   T50'` |  |



**FRU 文件示例：**

```yaml
ChassisInfo:
  part_number: AQ2U224AC      # 普通字符串，无需引号
  serial_number: ''            # 空字符串，必须加引号
  type: '23'                   # 希望保留为字符串而非数字
```

> **提示**：不加引号时，YAML 会尝试自动推断类型。`23` 解析为整数，`true` 解析为布尔值，`''` 表示空字符串。

---

## 五、列表（Sequences）

列表用短横线 `-` 开头，后面跟一个空格，每个元素占一行。

**FRU 文件示例：**

```yaml
MultirecordArea:
  - type: MgmtAccessRecord
    id: sys_unique_id
    guid: 74e4fee5-fde8-4095-bd2f-a7c21cd87cea

  - type: AquariusOEMRecord_v1
    macs:
      - 00 58 3F 1E 8F 82  # BMC dedicated port
      - 00 58 3F 1E 8F 83  # BMC shared port (NCSI)
      - 00 58 3F 1E 8F 84  # Host i225
```

解读：
- `MultirecordArea` 的值是一个列表，包含 2 个元素
- 每个 `-` 代表列表的一个元素
- 每个元素本身又是一个映射（包含 `type`、`id` 等键）
- `macs` 是一个嵌套列表 —— 列表中的列表

---

## 六、嵌套结构

YAML 通过**缩进层级**表达嵌套关系，可以任意深度嵌套。

```yaml
ProductInfo:                        # 第 1 层：顶层键
  manufacturer: Aquarius            # 第 2 层：ProductInfo 的子键
  part_number: 'T50 D224AC'
  serial_number: 225042105690J-0001
  asset_tag: ''
  version: ''
```

更复杂的嵌套（映射 → 列表 → 映射 → 列表）：

```yaml
MultirecordArea:           # 映射
  - type: AquariusOEMRecord_v1   # 列表元素（映射）
    macs:                        # 映射中的键
      - 00 58 3F 1E 8F 82       # 嵌套列表
      - 00 58 3F 1E 8F 83
```

层级关系用缩进可视化：

```
MultirecordArea          ← 第1层（键）
  -                      ← 第2层（列表元素）
    type: ...            ← 第3层（元素内的键值对）
    macs:                ← 第3层（键）
      - ...              ← 第4层（嵌套列表元素）
```

---

## 七、注释

用 `#` 添加注释，可以独占一行，也可以跟在值的后面。

**FRU 文件示例：**

```yaml
macs:
  - 00 58 3F 1E 8F 82  # BMC dedicated port
  - 00 58 3F 1E 8F 83  # BMC shared port (NCSI)
  - 00 58 3F 1E 8F 84  # Host i225
```

> **注意**：`#` 前面要有空格，否则可能被当作字符串的一部分。

---

## 八、数据类型速查

YAML 会自动推断常见数据类型：

| 写法 | 解析类型 | 示例 |
|------|----------|------|
| `Aquarius` | 字符串 | 无需引号的普通文本 |
| `'23'` | 字符串（强制） | 加引号保留为字符串 |
| `23` | 整数 | 不加引号的纯数字 |
| `3.14` | 浮点数 | 含小数点 |
| `true` / `false` | 布尔值 | 不加引号 |
| `''` | 空字符串 | 一对单引号 |
| `null` 或 `~` 或留空 | 空值 | 表示缺失 |
| `2026-09-16 05:44:58` | 日期时间 | ISO 8601 格式会自动识别 |

**FRU 文件中的类型体现：**

```yaml
mfg_date_time: 2026-09-16 05:44:58.963540   # 日期时间
type: '23'                                    # 字符串（非整数）
serial_number: ''                             # 空字符串
```

---

## 九、完整示例解析

下面是完整的 FRU 文件，融合了上述所有语法要素：

```yaml
# ===== 板卡信息 =====
BoardInfo:
  fru_file_id: ''                                    # 空字符串
  manufacturer: Aquarius                             # 普通字符串
  mfg_date_time: 2026-09-16 05:44:58.963540          # 日期时间
  part_number: AQC621AC-MB-12-01                     # 含连字符的字符串
  product_name: AQC621AC
  serial_number: AQ224070119

# ===== 机箱信息 =====
ChassisInfo:
  part_number: AQ2U224AC
  serial_number: ''                                  # 空字符串
  type: '23'                                         # 引号保留为字符串

# ===== 多记录区域（列表结构） =====
MultirecordArea:
  - type: MgmtAccessRecord                           # 列表第1个元素
    id: sys_unique_id
    guid: 74e4fee5-fde8-4095-bd2f-a7c21cd87cea

  - type: AquariusOEMRecord_v1                       # 列表第2个元素
    macs:
      - 00 58 3F 1E 8F 82  # BMC dedicated port      # 嵌套列表 + 行内注释
      - 00 58 3F 1E 8F 83  # BMC shared port (NCSI)
      - 00 58 3F 1E 8F 84  # Host i225

# ===== 产品信息 =====
ProductInfo:
  asset_tag: ''
  fru_file_id: ''
  manufacturer: Aquarius
  part_number: 'T50 D224AC'                         # 含空格，需引号
  product_name: T50 D224AC
  serial_number: 225042105690J-0001
  version: ''
```

---

## 十、常见陷阱

| 陷阱 | 错误写法 | 正确写法 |
|------|----------|----------|
| 冒号后缺空格 | `key:value` | `key: value` |
| 用 Tab 缩进 | `\tvalue` | 用空格缩进 |
| 空值不加引号 | `field:` | `field: ''`（如需空字符串） |
| 数字字符串不加引号 | `type: 23` | `type: '23'`（如需字符串） |
| 缩进不一致 | 混用 2 空格和 4 空格 | 全文统一缩进宽度 |

---

## 附：YAML 与 JSON 对比

```yaml
# YAML 写法
BoardInfo:
  manufacturer: Aquarius
  serial_number: AQ224070119
```

```json
// JSON 等价写法
{
  "BoardInfo": {
    "manufacturer": "Aquarius",
    "serial_number": "AQ224070119"
  }
}
```

YAML 相比 JSON：无需大括号和引号，用缩进代替嵌套，更简洁易读。
