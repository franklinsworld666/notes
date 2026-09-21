# sdbusplus 

> 目标不是记住所有 API，而是建立一条可重复使用的工作路径：**看见一个 D-Bus 对象 → 用 `busctl` 理解它 → 用 sdbusplus 调用或实现它 → 回到 OpenBMC 源码定位实现。**


---

## 1. 全景：先理解什么问题

OpenBMC 不是一个单体程序。网络、传感器、库存、主机状态、风扇控制等能力通常由不同进程实现；这些进程要互相读取状态、发出命令、通知变化。D-Bus 是它们之间最主要的 IPC（进程间通信）机制。

```text
phosphor-network  ──┐
entity-manager    ──┼── D-Bus broker ── bmcweb / IPMI / 其他服务
sensor 服务        ──┤
host-state-manager──┘
```

把 D-Bus 粗略看作“本机 RPC 总线”即可：Client 向 Server 发请求，Server 回复；同时 Server 还可以主动广播事件。它和 socket、pipe、共享内存并不互斥，只是提供了更统一的命名、类型、权限和观察方式。

`sdbusplus` 是 OpenBMC 常用的 C++ 封装库。它建立在 systemd 的 `sd-bus` 之上，负责把 C++ 类型、函数、回调与 D-Bus message 对接。它**不是 D-Bus 本身**，也不是消息中间件进程。

学习时牢记两条主线：

```text
Client：找服务 → 调用 Method / 读写 Property → 处理回复或错误
Server：申请服务名 → 导出 Object/Interface → 提供 Property/Method/Signal
```

## 2. D-Bus 的对象模型

理解 OpenBMC D-Bus 的关键是分清五个名词。

```text
Service（进程在总线上的名字）
  └── Object Path（对象的位置）
        └── Interface（对象实现的能力集合）
              ├── Property（有名称、可读取/写入的状态）
              ├── Method（可调用的操作）
              └── Signal（主动广播的事件）
```

例如，一个网络接口对象可能可抽象为：

```text
Service:      xyz.openbmc_project.Network
Object path:  /xyz/openbmc_project/network/eth0
Interface:    xyz.openbmc_project.Network.EthernetInterface
Property:     MACAddress, DHCPEnabled, Nameservers
Method:       （视具体接口而定）
Signal:       PropertiesChanged
```

这五者有不同职责：

| 概念          | 作用        | 类比              |
| ----------- | --------- | --------------- |
| Service     | 谁拥有、提供能力  | 某个服务进程或服务器      |
| Object path | 服务中的哪个对象  | 文件系统路径 / URL 路径 |
| Interface   | 对象提供哪一组契约 | C++ 接口或类的公开 API |
| Property    | 当前状态      | 成员变量的受控读写       |
| Method      | 让对象做一件事   | 成员函数            |
| Signal      | 状态或事件的通知  | 发布/订阅事件         |

注意两点：

- 一个 Service 可以拥有多个 Object；一个 Object 也可实现多个 Interface。
- Interface 并不等同于 Service。很多接口（如 `org.freedesktop.DBus.Properties`）是通用协议接口，任何服务都可以在自己的对象上实现它。

### Method、Property、Signal 的选择

| 需求 | 更合适的模型 | 原因 |
| --- | --- | --- |
| 读取当前温度、MAC 地址、运行状态 | Property | 有稳定名称的状态 |
| 请求重启主机、清除日志、执行校准 | Method | 表示一次动作，通常可有参数和返回值 |
| 通知温度跨阈值、对象新增或状态变化 | Signal | Server 主动广播，Client 不需要轮询 |

## 3. 用 busctl 观察正在运行的系统

在写一行 C++ 前，先用 `busctl` 验证真实系统上的对象模型。这是排查和读源码时最有效的起点。

```bash
# 列出 system bus 上的服务
busctl list

# 查看某服务导出了哪些对象
busctl tree xyz.openbmc_project.Network

# 查看对象上的接口、属性、方法和签名
busctl introspect \
  xyz.openbmc_project.Network \
  /xyz/openbmc_project/network/eth0

# 查看服务的 PID、凭据和总线名称等
busctl status xyz.openbmc_project.Network
```

`introspect` 的输出是最重要的输入。观察时依次回答：

1. **Service 是谁？** 是否已在总线中出现？
2. **Object path 是什么？** 路径是否精确存在？
3. **目标 Interface 是什么？** 不要把它和服务名混淆。
4. **Property 或 Method 名称是什么？** 大小写必须一致。
5. **参数和返回值 signature 是什么？**
6. **访问权限是什么？** Property 是只读、可写，还是需要特定权限？

常用命令：

```bash
# 读取一个属性
busctl get-property \
  xyz.openbmc_project.Network \
  /xyz/openbmc_project/network/eth0 \
  xyz.openbmc_project.Network.EthernetInterface \
  MACAddress

# 调用无参数 Method；最后的 Method 名称视实际接口而定
busctl call SERVICE PATH INTERFACE METHOD

# 调用带参数 Method，参数后的 s/b/u 等是 signature
busctl call SERVICE PATH INTERFACE METHOD s "argument"

# 监听总线流量；用于确认 Method 调用、回复或 Signal 是否真的发生
busctl monitor xyz.openbmc_project.Network
```

`busctl set-property` 需要按 D-Bus 类型提供值。例如，下面只是命令结构示意；务必先确认属性可写及实际 signature：

```bash
busctl set-property SERVICE PATH INTERFACE PROPERTY s "value"
```


## 4. sdbusplus 的位置与核心对象

```text
你的 C++ 应用
    │
    ├── sdbusplus::bus::bus         同步风格的总线连接
    ├── sdbusplus::asio::connection Asio 集成的异步连接
    └── sdbusplus::asio::object_server 运行时导出接口的便捷层
    │
sd-bus（libsystemd）
    │
D-Bus broker / dbus-daemon
```

最常见的三类对象：

```cpp
// 适合简单同步客户端、底层 message 操作
sdbusplus::bus::bus bus = sdbusplus::bus::new_default();

// 适合 OpenBMC 服务：D-Bus 与定时器、socket 共用一个事件循环
boost::asio::io_context io;
auto bus = std::make_shared<sdbusplus::asio::connection>(io);

// 在运行时创建 D-Bus interface（学习和小型服务很方便）
sdbusplus::asio::object_server objectServer(bus);

```


### Message 是什么

Method 调用、回复、错误、Signal 在总线上都是 message。sdbusplus 的底层接口使你能明确创建和填充消息：

```cpp
auto request = bus.new_method_call(service, path, interface, method);
request.append(argument1, argument2);
auto reply = bus.call(request);
reply.read(result1, result2);
```

这样读比记 API 更自然：**先构造请求 message，填入参数，发送，最后从 reply 读回返回值。**

## 5. 编写同步 Client：调用 Method

下面是一个同步调用的骨架。请把常量替换为你由 `busctl introspect` 获得的真实值。

```cpp
#include <sdbusplus/bus.hpp>

#include <iostream>
#include <string>

int main()
{
    constexpr const char* service = "xyz.openbmc_project.Example";
    constexpr const char* path = "/xyz/openbmc_project/example";
    constexpr const char* interface = "xyz.openbmc_project.Example.Control";
    constexpr const char* methodName = "Echo";

    auto bus = sdbusplus::bus::new_default();
    auto request = bus.new_method_call(service, path, interface, methodName);
    request.append(std::string{"hello"});

    try
    {
        auto reply = bus.call(request);
        std::string echoed;
        reply.read(echoed);
        std::cout << echoed << '\n';
    }
    catch (const sdbusplus::exception::exception& e)
    {
        std::cerr << "D-Bus call failed: " << e.what() << '\n';
        return 1;
    }
}
```

执行链路如下：

```text
new_method_call()
       ↓
MethodCall message（含 Service、Path、Interface、Method）
       ↓ append()
带上请求参数
       ↓ call()
将请求发往 D-Bus，并阻塞等待
       ↓
目标服务的 Method handler 执行
       ↓
reply.read()
按返回 signature 解码结果
```

同步调用很直观，但不能滥用。若程序在单线程事件循环中处理 HTTP、定时器、传感器或大量总线请求，阻塞等待某个服务的回复会让整个循环停住。此时应该使用异步调用，见第 8 章。

## 6. 参数、返回值与 D-Bus signature

D-Bus 通过 signature 描述 message 的序列化类型。它相当于跨进程 API 的类型契约；`busctl introspect` 输出的 `in`、`out` 列就是可靠来源。

常见基本类型：

| Signature | 含义 | 常见 C++ 类型 |
| --- | --- | --- |
| `s` | string | `std::string` |
| `b` | boolean | `bool` |
| `i` / `u` | 32 位有符号 / 无符号整数 | `int32_t` / `uint32_t` |
| `x` / `t` | 64 位有符号 / 无符号整数 | `int64_t` / `uint64_t` |
| `d` | double | `double` |
| `o` | object path | 通常读作 `sdbusplus::message::object_path` 或字符串（取决于接口定义） |
| `g` | signature | `sdbusplus::message::signature` |
| `v` | variant | `std::variant<...>` |

常见容器类型：

| Signature | 含义 | 常见 C++ 类型示意 |
| --- | --- | --- |
| `as` | string 数组 | `std::vector<std::string>` |
| `a{sv}` | `string → variant` 字典 | `std::map<std::string, std::variant<...>>` |
| `a(ss)` | `(string, string)` 数组 | `std::vector<std::tuple<std::string, std::string>>` |

### `append()` 和 `read()` 必须与 signature 对齐

若 Method 的输入是 `su`，输出是 `b`：

```cpp
std::string name{"eth0"};
uint32_t index = 0;
request.append(name, index); // 对应 s + u

auto reply = bus.call(request);
bool success{};
reply.read(success);         // 对应 b
```

最常见错误不是“D-Bus 很难”，而是参数顺序、C++ 整数宽度或 `variant` 候选类型与实际 signature 不匹配。不要猜类型；从 introspection 和 YAML 定义确认。

### 为什么 `a{sv}` 在 OpenBMC 如此常见

`a{sv}` 表示一个属性字典：键是属性名，值是可承载多种类型的 `variant`。`GetAll`、ObjectMapper 返回值、接口配置常使用它。

```cpp
using PropertyValue = std::variant<std::string, bool, uint32_t, int64_t>;
using Properties = std::map<std::string, PropertyValue>;
```

实际业务使用的 `variant` 候选集应该尽量精确，且需要覆盖对端真实属性类型。随后用 `std::get_if<T>` 安全提取：

```cpp
if (const auto* mac = std::get_if<std::string>(&properties.at("MACAddress")))
{
    std::cout << *mac << '\n';
}
```

## 7. Property：Get、GetAll、Set

Property 不是“直接访问远端成员变量”；它通过标准接口 `org.freedesktop.DBus.Properties` 暴露。对任何具体业务接口的属性操作，本质上都调用这个标准接口的 Method。

### Get：读取一个属性

```cpp
#include <sdbusplus/bus.hpp>

#include <string>
#include <variant>

auto bus = sdbusplus::bus::new_default();
auto request = bus.new_method_call(
    service, path, "org.freedesktop.DBus.Properties", "Get");
request.append(std::string{targetInterface}, std::string{"MACAddress"});

auto reply = bus.call(request);
std::variant<std::string> value;
reply.read(value);

const auto& mac = std::get<std::string>(value);
```

`Get` 的请求参数是 **目标业务 Interface 名**和 **Property 名**。它的回复是一个 `variant`，即便你的属性本身只是字符串。

### GetAll：读取一个 Interface 的全部属性

```cpp
using PropertyValue = std::variant<std::string, bool, uint32_t>;
using Properties = std::map<std::string, PropertyValue>;

auto request = bus.new_method_call(
    service, path, "org.freedesktop.DBus.Properties", "GetAll");
request.append(std::string{targetInterface});

auto reply = bus.call(request);
Properties properties;
reply.read(properties); // 对应 a{sv}
```

`GetAll` 能少一次往返，但也可能取回你不需要的数据。对变化频繁、对象很多的场景，结合缓存和 `PropertiesChanged` 信号通常更合适。

### Set：写入属性

`Set` 的参数顺序是：目标 Interface、Property 名、包装后的 `variant`。

```cpp
using Property = std::variant<bool>;

auto request = bus.new_method_call(
    service, path, "org.freedesktop.DBus.Properties", "Set");
request.append(std::string{targetInterface},
               std::string{"DHCPEnabled"},
               Property{true});

bus.call(request);
```

写属性前需要确认：

- 属性在 introspection 中是否可写；
- 对端 setter 是否接受该值；
- 当前调用者的 D-Bus policy / 权限是否允许；
- 这个状态是否应使用 Method 表达。例如“重置系统”不应伪装成 `Reset=true` 的 Property。

## 8. 异步调用与执行时序

OpenBMC 服务大多是事件驱动的。异步调用不会在发出请求后阻塞当前线程，而是在回复抵达时调度回调。

```cpp
bus->async_method_call(
    [](const boost::system::error_code& ec, const std::string& value) {
        if (ec)
        {
            std::cerr << "call failed: " << ec.message() << '\n';
            return;
        }
        std::cout << "reply: " << value << '\n';
    },
    service, path, interface, methodName, requestArgument);
```

同一过程与同步版本的差别：

```text
同步 call()：
发送请求 ──> 等待回复 ──> 解码结果 ──> 继续执行

异步 async_method_call()：
发送请求 ──> 立即返回 ──> 事件循环继续处理其他工作
                                  │
回复到达 ──────────────────────────┘
                                  ↓
                             执行 callback
```

以下代码的“B”通常会先打印：

```cpp
bus->async_method_call(callback, service, path, interface, method);
std::cout << "B\n";
```

### 必须建立的四个认知

1. 调用 `async_method_call()` 时请求会被提交，但回调稍后由事件循环运行。
2. 回调在收到正常回复或错误时执行，错误以 `boost::system::error_code` 体现。
3. 两个请求的回调顺序由对端响应时机决定，**不保证与发起顺序一致**。
4. 若 `io_context.run()` 未运行，回调不会被处理；见下一章。

不要在函数返回后假设异步结果已可用：

```cpp
// 错误示意：result 在 callback 执行前仍是默认值。
std::string result;
bus->async_method_call([&result](auto ec, std::string reply) {
    if (!ec) result = std::move(reply);
}, service, path, interface, method);
use(result); // 太早
```

正确做法是让“依赖结果的下一步”在 callback 中触发，或使用一个明确的异步组合机制。

## 9. 多个异步调用的组合

假设页面或业务逻辑需要 MAC、IP、链路状态都完成后再继续。不能把三个 `async_method_call()` 后面的下一行当作“全部完成”。

```text
Start
  ├── Get MAC ──────┐
  ├── Get IP ───────┼── 三个回调都成功/结束后 → Continue
  └── Get Link ─────┘
```

一种容易读懂的计数器模式如下。真实项目还应根据业务选择“任一失败即停止”或“收集所有错误再返回”。

```cpp
struct NetworkInfo
{
    std::string mac;
    std::string ip;
    bool link{};
};

struct Pending
{
    NetworkInfo info;
    int remaining = 3;
    bool failed = false;
};

auto state = std::make_shared<Pending>();
auto finishOne = [state](const boost::system::error_code& ec) {
    if (ec)
    {
        state->failed = true;
    }
    if (--state->remaining != 0)
    {
        return;
    }

    if (state->failed)
    {
        // 统一失败处理
        return;
    }
    // 这里三个结果都已写入 state->info
    // continue(state->info);
};
```

每个回调捕获 `state`，在写入各自的字段后调用 `finishOne(ec)`。`shared_ptr` 的意义是让共享状态活到最后一个回调结束；它不是解决所有生命周期问题的万能药，见第 17 章。

复杂异步流程可按项目规范使用以下方式之一：

- 小流程：回调 + 清晰的共享状态；
- 有顺序依赖的流程：把下一步放在上一回调中，控制嵌套层数；
- 支持协程的代码：用 `co_await` 表达顺序逻辑；
- 需要取消、超时、扇入/扇出：使用项目已有的异步抽象，避免自制难以维护的框架。

## 10. Boost.Asio 与事件循环

`sdbusplus::asio::connection` 将 D-Bus 文件描述符纳入 Boost.Asio 的 `io_context`。同一个事件循环可以处理 D-Bus 回复、定时器、socket、HTTP 等事件。

```cpp
#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>

int main()
{
    boost::asio::io_context io;
    auto bus = std::make_shared<sdbusplus::asio::connection>(io);

    // 注册异步 D-Bus 调用、定时器或对象服务。

    io.run(); // 事件循环：分派 callback，直到没有未完成工作
}
```

```text
io_context
   ├── D-Bus response → async_method_call 的 callback
   ├── D-Bus signal   → match 的 callback
   ├── Timer           → wait handler
   └── Socket/HTTP     → 对应 handler
```

### `bus::bus` 和 `asio::connection` 如何选择

| 情况 | 通常选择 |
| --- | --- |
| 一次性命令行工具，调用后退出 | `sdbusplus::bus::bus` + 同步调用 |
| 服务端、守护进程、bmcweb 风格程序 | `sdbusplus::asio::connection` |
| 需要 D-Bus、定时器、网络共同驱动 | `sdbusplus::asio::connection` |

异步程序的常见误区是“注册了 callback 却什么也没发生”。优先检查 `io.run()` 是否真的开始执行、`io_context` 是否被提前停止，以及连接和注册对象是否仍然存活。

## 11. 编写 D-Bus Server

一个最小 Server 需要做四件事：

1. 创建 Asio 连接；
2. 申请一个唯一 Service 名；
3. 在 Object path 上添加 Interface；
4. 注册能力后初始化，并运行事件循环。

```cpp
#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <memory>

int main()
{
    boost::asio::io_context io;
    auto bus = std::make_shared<sdbusplus::asio::connection>(io);
    bus->request_name("xyz.openbmc_project.Example");

    sdbusplus::asio::object_server server(bus);
    auto iface = server.add_interface(
        "/xyz/openbmc_project/example",
        "xyz.openbmc_project.Example.Control");

    // register_property / register_method / register_signal
    iface->initialize();
    io.run();
}
```

`request_name()` 成功只是表明进程在总线上拥有该名字；真正能被 Client 访问还依赖你导出了正确的对象路径与接口。`initialize()` 前完成所有注册，避免 Client 看到不完整的接口。

> 上述 `object_server` 适合理解模型、实现动态接口或轻量服务。OpenBMC 中公共、稳定的 D-Bus API 通常进一步由 YAML 定义并生成 server/client 代码，见第 15 章。

## 12. Property、Method、Signal 的服务端实现

### 注册 Property

```cpp
std::string status{"Ready"};
iface->register_property("Status", status);
```

可读写属性通常还要在 setter 中校验输入、更新内部状态，并决定是否接受。具体 `register_property` 的重载和 flags 取决于项目使用的 sdbusplus 版本；以代码库已有模式和生成接口为准。

重要原则：**Property 的值应来自服务的真实状态，而不是仅为应付 D-Bus 保存一份脱节副本。** 状态变化后还应以规范的方式通知订阅者。

### 注册 Method

```cpp
iface->register_method("Echo", [](const std::string& input) {
    return input;
});
```

这个 lambda 的参数和返回类型形成 Method 的 D-Bus 类型契约。对于业务操作，应明确：

- 输入是否需要校验；
- 操作是否可能耗时；
- 是立即完成还是只接受请求后异步推进；
- 失败时应返回哪种标准错误；
- 是否需要权限检查。

### Signal 与 `PropertiesChanged`

普通 Signal 是 Server 广播的事件；Method 的调用者不等于 Signal 的接收者。多个 Client 可以订阅同一 Signal。

```text
Server
  └── Signal ──> Client A
          └──> Client B
          └──> Client C
```

属性变化时，Client 通常关心标准的 `org.freedesktop.DBus.Properties.PropertiesChanged` 信号。使用 `object_server` 的运行时接口时，常见形式是更新属性后通知变更：

```cpp
// 具体 API 以当前 sdbusplus 版本为准。
iface->set_property("Status", std::string{"Running"});
```

若使用 generated server，应优先调用生成的 property setter；它通常同时完成状态更新和必要的 D-Bus 通知。不要为了“保险”手工重复发送变更信号，否则 Client 可能收到重复事件。

## 13. 错误处理与排查路径

同步风格通常捕获 `sdbusplus::exception::exception`；异步风格在 callback 的 `boost::system::error_code` 中检查失败。无论风格如何，日志应该包含目标四元组：**Service、Path、Interface、Method/Property**，以及错误文本。

常见错误与首要检查点：

| 错误 | 常见原因 | 首先执行的检查 |
| --- | --- | --- |
| `ServiceUnknown` | 服务未启动、名称写错 | `busctl list` |
| `UnknownObject` | Object path 不存在 | `busctl tree SERVICE` |
| `UnknownInterface` | Interface 名错误或未导出 | `busctl introspect SERVICE PATH` |
| `UnknownMethod` | Method 名错误或版本不匹配 | `busctl introspect` 的 methods 列 |
| `InvalidArgs` | 参数数量、顺序、signature 不匹配 | 对照 `in` signature 与 `append()` |
| `AccessDenied` | D-Bus policy 或调用者权限不足 | policy、服务日志、调用身份 |
| timeout / 无回复 | 对端卡住、事件循环没跑、依赖链阻塞 | `busctl monitor`、服务日志、systemd 状态 |

固定排查顺序可以避免在 C++ 代码中盲猜：

```text
Service 是否存在
  ↓
Object Path 是否存在
  ↓
Interface 是否存在
  ↓
Method / Property 名称是否存在
  ↓
参数、返回值 signature 是否一致
  ↓
权限、状态、依赖服务是否满足
```

### 一个高效的现场诊断流程

```bash
busctl list | grep xyz.openbmc_project
busctl tree SERVICE
busctl introspect SERVICE PATH
busctl get-property SERVICE PATH INTERFACE PROPERTY
busctl monitor SERVICE
systemctl status 对应服务名
journalctl -u 对应服务名
```

先在命令行验证对象模型，再对照源码中的常量、`async_method_call` 参数及 C++ 类型。这样可把“D-Bus 不通”的问题迅速收敛为命名、类型、生命周期或权限问题。

## 14. ObjectMapper 与动态服务发现

OpenBMC 常把对象的“服务归属”交给 ObjectMapper 查询，而不是在 Client 中硬编码 Service 名。原因是同一个接口或对象路径可能由不同实现、不同配置或不同平台服务提供；硬编码会让模块耦合过紧。

ObjectMapper 的常用服务是：

```text
xyz.openbmc_project.ObjectMapper
```

常见 Method：

| Method | 用途 |
| --- | --- |
| `GetObject` | 已知对象路径和所需接口，找提供它的服务 |
| `GetSubTree` | 在某个根路径下按深度和接口筛选对象及服务 |
| `GetSubTreePaths` | 只取匹配对象路径 |
| `GetAssociatedSubTree` | 沿 association 找关联对象，再筛选 |

典型流程：

```text
已知：Object Path + 需要的 Interface
        ↓
调用 ObjectMapper.GetObject
        ↓
得到：Service → [它在该对象上提供的 Interface 列表]
        ↓
对返回的 Service 发起 Get / Method 调用
```

`GetObject` 返回值通常类似 `a{sas}`：服务名映射到接口名数组。实际的 C++ 类型形状会是嵌套 `map<string, vector<string>>` 一类容器；请以该接口 YAML/`busctl introspect` 为准。

一个常见错误是将 ObjectMapper 当作业务数据服务。它回答的是“哪个服务实现了哪个对象/接口”，不是“该对象的属性值是什么”。找到服务后，还要向真正的业务 Service 调用 Properties 或业务 Method。

## 15. YAML 与 sdbusplus 代码生成

大量 OpenBMC D-Bus 接口并非手写 C++ 字符串定义，而是先用 YAML 描述 API，再由 sdbusplus 的生成工具产出 C++ 代码和文档。

```text
Interface YAML
     ↓
sdbusplus code generation
     ↓
generated server / client / enum / exception header
     ↓
业务服务继承或组合 generated server
```

YAML 作为 API 合约通常描述：

- Interface 名称与描述；
- Property 的类型、访问权限、默认值；
- Method 的输入、输出与错误；
- Signal 参数；
- enum；
- exception / error。

生成代码的意义不仅是减少样板代码，更重要的是让服务端、客户端、D-Bus introspection 与类型定义来自同一份契约。源码中看到类似下面的命名空间时，应优先想到“这是生成接口的一部分”：

```cpp
xyz::openbmc_project::Example::server::Control
```

### 动态接口与生成接口如何取舍

| 需求 | 通常更合适的做法 |
| --- | --- |
| 临时工具、实验、小型动态对象 | `asio::object_server` / `dbus_interface` |
| 产品中稳定、跨服务使用的公开 API | YAML + generated server/client |
| 只需要调用已存在接口 | 使用对应 generated client 或直接 sdbusplus Client 调用 |

不要直接编辑生成文件。应修改源 YAML 或业务实现，再按照项目的构建/生成流程更新产物。

## 16. 从 D-Bus 追踪回 OpenBMC 源码

阅读 OpenBMC 代码时，建议从运行时事实反向定位，而不是从大量仓库中盲搜。

```text
busctl introspect
       ↓
Service / Object Path / Interface / Property 或 Method
       ↓
systemctl status（确认实际进程与 unit）
       ↓
仓库中搜索 Service 名、对象路径、Interface 名或 YAML 名
       ↓
找到 request_name / add_interface / generated server 继承点
       ↓
沿 property setter、method handler、signal 追踪业务逻辑
```

### 推荐的搜索顺序

1. 搜 Interface 全名（例如 `xyz.openbmc_project.*`）。这常能找到 YAML、生成依赖或实现。
2. 搜 Object path 前缀（例如 `/xyz/openbmc_project/...`）。这有助于找到对象构造处。
3. 搜 Service 名和 `request_name`。这有助于锁定拥有者。
4. 搜 Property / Method 名。在结果过多时结合类名、模块目录、Interface 名缩小范围。
5. 如果服务名不是固定的，先查看 ObjectMapper 相关调用和配置。

### 三条真实业务链应如何理解

```text
Sensor：硬件/驱动数据 → D-Bus Value Property → Redfish / IPMI / Web

Network：配置或网络事件 → network service 的对象和属性 → bmcweb / CLI

State：状态请求 → xyz.openbmc_project.State.* Method/Property → systemd 或状态机动作
```

实体管理相关对象常有动态特征：配置出现、硬件被发现或平台描述变化时，Object/Interface 可能新增、移除、属性更新。因此 Client 不应只在启动时查询一次就永远假设对象存在。

## 17. 信号、生命周期与并发安全

### 订阅信号

Client 可使用 match rule 订阅指定 Service、Path、Interface、Member 的 signal。常见关注点包括：

- `PropertiesChanged`：属性变化；
- `InterfacesAdded` / `InterfacesRemoved`：对象接口增删；
- `NameOwnerChanged`：服务出现、消失或重启。

信号是**观察变化**，不是可靠的状态数据库。一个健壮的 Client 通常这样做：

```text
1. 初始 Get / GetAll，得到当前快照
2. 安装 signal match
3. 收到变化后更新本地缓存
4. 服务重启、对象移除时清理或重新发现
```

安装 match 与读初始值之间可能存在竞态，具体处理方式要依据业务的丢失容忍度设计；高可靠场景应能在重连或重新枚举时恢复正确状态。

### `shared_ptr`、`weak_ptr` 与 lambda capture

异步 callback 执行时，发起调用的对象可能已经析构。下面的 `[this]` 有悬空指针风险：

```cpp
bus->async_method_call([this](auto ec, auto&& reply) {
    updateFromReply(reply); // 若 *this 已销毁，行为未定义
}, service, path, interface, method);
```

若对象以 `std::shared_ptr` 管理，较稳妥的模式是捕获 `weak_ptr`，回调中先锁定：

```cpp
std::weak_ptr<Controller> weakSelf = shared_from_this();
bus->async_method_call([weakSelf](auto ec, const auto& reply) {
    auto self = weakSelf.lock();
    if (!self || ec)
    {
        return;
    }
    self->updateFromReply(reply);
}, service, path, interface, method);
```

不要为了“避免析构”无节制捕获 `shared_ptr<this>`，否则对象可能被回调、timer、match 互相引用而延长到意料之外的生命周期。选择 `shared_ptr` 或 `weak_ptr` 前先明确谁拥有对象、何时允许请求取消。

### 服务重启和超时

在 BMC 上，依赖服务可能启动较慢、崩溃重启或暂时不可用。健壮代码应：

- 把 `ServiceUnknown`、无对象、超时视为可预期运行态，而非必然程序错误；
- 在合适范围内重试，并设置退避，避免失败时打爆总线；
- 关注 NameOwnerChanged 或接口增删信号；
- 不缓存永远有效的 Service 归属；必要时重新查询 ObjectMapper；
- 避免在 D-Bus callback 中做长时间阻塞工作。

### 性能边界

高频更新的 PropertyChanged、大量对象、成批 `GetAll`、短时间内海量异步请求都会影响总线和事件循环。工程上优先：

- 按需订阅与查询；
- 合并重复请求；
- 缓存可缓存的状态；
- 对变化去抖或节流；
- 在日志中避免对每次正常状态变化输出高等级信息。

## 18. 练习路线与速查表

### 建议按这个顺序练习

1. 在目标 BMC 上用 `busctl list`、`tree`、`introspect` 找一个你关心的对象。
2. 用 `get-property` 读取一个 Property，并写下 Service/Path/Interface/Property 四元组。
3. 根据 introspection 的 signature，写一个同步 `Get` 或业务 Method Client。
4. 将它改为 `async_method_call()`，观察 `io.run()` 前后回调的区别。
5. 用 `object_server` 写一个有一个 Property 和一个 Echo Method 的小服务，用 `busctl` 验证它。
6. 订阅或监控一次 `PropertiesChanged`，让服务端修改属性并观察 Client。
7. 对一个真实对象使用 ObjectMapper 发现 Service，再读取属性。
8. 从该对象的 Interface 名反查 YAML、生成类与业务 handler。

### 最小检查清单

当你拿到一个 D-Bus 需求时，先填完下面这张卡片：

```text
目标：读取 / 写入 / 执行 / 订阅什么？
Service：
Object path：
业务 Interface：
成员名（Property / Method / Signal）：
输入 signature：
输出 signature：
是否需要 ObjectMapper：
同步还是异步：
失败、服务重启、对象消失时怎么办：
如何用 busctl 验证：
```

### API 速查

| 任务        | 关键词 / API                                                                                  |
| --------- | ------------------------------------------------------------------------------------------ |
| 查看系统对象    | `busctl list/tree/introspect/monitor`                                                      |
| 同步 Client | `new_default()`、`new_method_call()`、`append()`、`call()`、`read()`                           |
| 异步 Client | `sdbusplus::asio::connection`、`async_method_call()`、`io_context.run()`                     |
| 单个属性      | `org.freedesktop.DBus.Properties.Get`                                                      |
| 所有属性      | `org.freedesktop.DBus.Properties.GetAll`                                                   |
| 写属性       | `org.freedesktop.DBus.Properties.Set`                                                      |
| 动态 Server | `object_server`、`add_interface()`、`register_property()`、`register_method()`、`initialize()` |
| 服务发现      | `xyz.openbmc_project.ObjectMapper`、`GetObject`、`GetSubTree`                                |
| 稳定接口定义    | YAML、sdbusplus code generation                                                             |

## 结语：把每次 D-Bus 交互还原成四元组

初学者最容易把 sdbusplus 看成一串复杂模板和回调。实际上，绝大部分问题都能还原成：

```text
谁（Service）
在什么对象上（Object Path）
通过哪个契约（Interface）
操作或观察什么成员（Property / Method / Signal）
并使用什么类型（Signature）
```

先用 `busctl` 验证这套事实，再用 sdbusplus 让 C++ 表达同一件事；当服务归属不固定时，用 ObjectMapper；当代码复杂时，把异步、生命周期与重启当作正常运行条件来设计。掌握这条路径后，阅读 `entity-manager`、网络服务、传感器服务和状态管理模块时，就能从一个总线对象稳定地追到实现代码。
