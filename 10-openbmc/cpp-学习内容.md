| 优先级 | C++学习内容                     | 达到标准                                       | OpenBMC中的典型场景                   | 参考资料                            |
| --- | --------------------------- | ------------------------------------------ | ------------------------------- | ------------------------------- |
| 1   | Lambda表达式                   | 能看懂 `[&]`、`[=]`、`[this]`、`[](auto& msg){}` | D-Bus回调、Asio回调                  | Effective Modern C++ Item 31-34 |
| 2   | 智能指针（unique_ptr/shared_ptr） | 理解所有权、引用计数、move                            | connection、sensor、object_server | C++ Primer 第12章                 |
| 3   | auto与类型推导                   | 能根据上下文推导实际类型                               | 几乎所有OpenBMC代码                   | Effective Modern C++ Item 1-6   |
| 4   | STL容器                       | 熟悉 vector/map/unordered_map/set            | inventory缓存、sensor缓存            | C++ Primer 第9章、第11章             |
| 5   | RAII思想                      | 理解构造注册资源、析构释放资源                            | match_t、timer、DBus对象            | C++ Core Guidelines R.1         |
| 6   | 回调机制（Callback）              | 理解函数对象与异步回调                                | async_method_call               | Effective Modern C++            |
| 7   | std::variant                | 能读懂 `get<>`、`holds_alternative`            | D-Bus Property Value            | cppreference variant            |
| 8   | std::visit                  | 能看懂 visit+lambda                           | 属性类型分发                          | cppreference visit              |
| 9   | std::optional               | 能读懂 optional 判断和取值                         | 可选配置、传感器属性                      | cppreference optional           |
| 10  | 引用与移动语义                     | 理解左值、右值、move                               | 高性能对象传递                         | Effective Modern C++ Item 23-25 |
| 11  | Sdbus                       | Sdbusplus 使用方法                             |                                 |                                 |
| 11  | Boost.Asio基础                | 理解 io_context、timer、event loop             | OpenBMC异步框架核心                   | Boost.Asio Tutorial             |
| 12  | std::function               | 理解函数对象封装                                   | 回调注册                            | cppreference function           |
| 13  | constexpr                   | 能看懂编译期常量                                   | 配置表、路径定义                        | cppreference constexpr          |
| 14  | enum class                  | 能读懂强类型枚举                                   | 状态机、设备状态                        | cppreference enum class         |
| 15  | decltype                    | 能推导表达式类型                                   | 模板代码、sdbusplus                  | Effective Modern C++            |
| 16  | 类型萃取(type_traits)           | 能看懂 `is_same_v` 等                          | sdbusplus模板                     | cppreference type_traits        |
| 17  | 完美转发(forward)               | 理解 `T&&` + forward                         | DBus接口封装                        | Effective Modern C++            |
| 18  | 模板偏特化/SFINAE                | 能看懂 `enable_if_t`                          | sdbusplus生成代码                   | C++ Templates                   |
| 19  | CRTP模板模式                    | 能看懂模板继承                                    | sdbusplus接口基类                   | C++ Templates                   |
| 20  | 模板元编程                       | 能勉强跟踪模板展开                                  | sdbusplus最底层实现                  | C++ Templates                   |