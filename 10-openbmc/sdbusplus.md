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


## 4. sdbusplus 的核心对象

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

// 适合异步：D-Bus 与定时器、socket 共用一个事件循环
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

| Signature | 含义                    | 常见 C++ 类型示意                                         |
| --------- | --------------------- | --------------------------------------------------- |
| `as`      | string 数组             | `std::vector<std::string>`                          |
| `a{sv}`   | `string → variant` 字典 | `std::map<std::string, std::variant<...>>`          |
| `a(ss)`   | `(string, string)` 数组 | `std::vector<std::tuple<std::string, std::string>>` |

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


## 7. 异步调用与执行时序

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

## 8. 多个异步调用的组合

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
    // must be atomic operator，or may be wrong
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

## 9. Boost.Asio 与事件循环

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

## 10. 编写 D-Bus Server

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
    // 创建 asio 连接
    boost::asio::io_context io;
    auto bus = std::make_shared<sdbusplus::asio::connection>(io);
    // 申请服务名
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

### 注册 Property

```cpp
std::string status{"Ready"};
iface->register_property("Status", status);
```

可读写属性通常还要在 setter 中校验输入、更新内部状态，并决定是否接受。具体 `register_property` 的重载和 flags 取决于项目使用的 sdbusplus 版本；以代码库已有模式和生成接口为准。

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

服务端：属性改变时发 `PropertiesChanged`

对于 `object_server` 的运行时属性，注册后直接 `set_property()` 即可。属性真实发生变化时，当前实现会自动发出标准的 `org.freedesktop.DBus.Properties.PropertiesChanged`，不用再手工调用 `signal_property()`。

```
auto iface = server.add_interface(kPath, kIface);

// 注册阶段：在 initialize 前
iface->register_property("Status", std::string{"Stopped"});
iface->register_property("Progress", uint32_t{0});
iface->initialize();

// 业务运行阶段：会自动发 PropertiesChanged
iface->set_property("Status", std::string{"Running"});
iface->set_property("Progress", uint32_t{42});
```

不要这样重复通知：

```
iface->set_property("Status", std::string{"Running"});
iface->signal_property("Status"); // 不要：可能造成重复 PropertiesChanged
```

`set_property` 对未变化的值默认不会再广播属性变化。运行时 `object_server` 的默认属性也带有 `emits_change` 标志。

客户端：订阅 `PropertiesChanged`

`PropertiesChanged` 的 D-Bus 签名是：`sa{sv}as`

分别是：

1. 发生变化的接口名；
2. `属性名 -> 新值`；
3. 已失效但未携带新值的属性名。

```
#include <boost/asio/io_context.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus/match.hpp>

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

constexpr char kPath[] = "/xyz/openbmc_project/demo";
constexpr char kWatchedIface[] = "xyz.openbmc_project.Demo.Status";

// 必须覆盖目标接口可能随信号携带的全部属性类型。
// 实际项目中应按接口定义收窄或扩展。
using DbusValue = std::variant<
    bool,
    int16_t, uint16_t,
    int32_t, uint32_t,
    int64_t, uint64_t,
    double,
    std::string>;

using ChangedProperties = std::map<std::string, DbusValue>;

int main()
{
    boost::asio::io_context io;
    auto bus = std::make_shared<sdbusplus::asio::connection>(io);

    // match 必须长期存活；若在局部临时对象中创建，会立刻取消订阅。
    sdbusplus::bus::match_t propertiesChangedMatch(
        *bus,
        sdbusplus::bus::match::rules::propertiesChanged(
            kPath, kWatchedIface),
        [](sdbusplus::message_t& msg) {
            std::string changedInterface;
            ChangedProperties changed;
            std::vector<std::string> invalidated;

            msg.read(changedInterface, changed, invalidated);

            // 只处理 Status
            auto iter = changed.find("Status");
            if (iter != changed.end())
            {
                if (const auto* status =
                        std::get_if<std::string>(&iter->second))
                {
                    std::cout << "Status changed: "
                              << *status << '\n';
                }
            }

            // invalidated 中的属性没有携带值，应通过 Properties.Get / GetAll 重新读取。
            for (const auto& property : invalidated)
            {
                std::cout << "Property invalidated: "
                          << property << '\n';
            }
        });

    io.run();
}
```

`propertiesChanged(kPath, kWatchedIface)` 已限定：

```
type=signal
path=/xyz/openbmc_project/demo
interface=org.freedesktop.DBus.Properties
member=PropertiesChanged
arg0=xyz.openbmc_project.Demo.Status
```

因此不会收到其他对象路径、其他接口的属性变化。


## 11. 错误处理与排查路径

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

## 12. ObjectMapper 与动态服务发现

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



## 13. 信号、生命周期与并发安全

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


