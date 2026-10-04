## Function

```
std::function
    ↓
解决“如何保存一个函数”, 它是一个可以保存各种 Callable 的通用函数容器。

std::bind
    ↓
解决“如何修改一个函数的参数”，提前固定部分参数



#include <functional>
#include <iostream>

int add(int a, int b)
{
    return a + b;
}

int main()
{
    std::function<int(int, int)> func = add;

    std::cout << func(10, 20) << '\n';


    std::function<int(int, int)> func2 =
        [](int a, int b)
        {
            return a + b;
        };

    std::cout << func2(10, 20) << '\n';

    return 0;
}
```

## Bind

```cpp
#include <functional>
#include <iostream>

int add(int a, int b)
{
    return a + b;
}

int main()
{
    auto func =
        std::bind(
            add,
            10,
            std::placeholders::_1);

    std::cout << func(20) << '\n';

    return 0;
}
```



## Lambda
```
#include <iostream>

int main()
{
    int value = 10;
    std::cout << "Initial value = " << value << "\n\n";

    // 1. 按值捕获
    // Lambda 内部保存 value 的一份副本
    auto captureByValue = [value]()
    {
        std::cout << "[capture by value]\n";
        std::cout << "inside value = " << value << "\n\n";
    };

    value = 20;
    std::cout << "outside value = " << value << "\n";

    captureByValue();

    // 2. 按引用捕获
    // Lambda 内部直接使用外部的 value
    auto captureByReference = [&value]()
    {
        std::cout << "[capture by reference]\n";
        std::cout << "before: value = " << value << "\n";
        value = 30;
        std::cout << "after : value = " << value << "\n";
    };
    captureByReference();

    std::cout << "outside value = " << value << "\n\n";

    // 3. 按值捕获 + mutable
    // 默认情况下，按值捕获的变量在 Lambda 内不能修改。
    // mutable 允许修改 Lambda 内部保存的副本。
    auto captureByValueMutable = [value]() mutable
    {
        std::cout << "[capture by value + mutable]\n";
        std::cout << "before: value = " << value << "\n";
        value = 40;
        std::cout << "after : value = " << value << "\n";
    };

    captureByValueMutable();

    // 外部 value 不会受到影响
    std::cout << "outside value = " << value << "\n";

    return 0;

}
```


## Unique_ptr

```
#include <iostream>
#include <memory>
#include <string>

class Person
{
public:
    explicit Person(const std::string& name) : name_(name)
    {
        std::cout << "[Create] " << name_ << std::endl;
    }

    ~Person()
    {
        std::cout << "[Destroy] " << name_ << std::endl;
   }

    void sayHello() const
    {
        std::cout << "Hello, I am " << name_ << std::endl;
    }

private:
    std::string name_;
};

// ============================================================
// 规则 1：按值接收 unique_ptr
// 表示“接管所有权”
// 调用时通常需要 std::move(ptr)
// ============================================================
void takeOwnership(std::unique_ptr<Person> p)
{
    // 函数结束后，p 被销毁
    // 它管理的 Person 也会自动释放    
    std::cout << "\n[takeOwnership]" << std::endl;
    p->sayHello();
}
 
// ============================================================
// 规则 2：const unique_ptr&
// 不转移所有权，只读取 unique_ptr，不能修改 p
// ============================================================
void inspectUniquePtr(const std::unique_ptr<Person>& p)
{
    std::cout << "\n[inspectUniquePtr]" << std::endl;
    if (p)
    {
        p->sayHello();
    }
}

// ============================================================
// 规则 3：unique_ptr&
// 不转移所有权，但可以修改 unique_ptr 本身
// ============================================================
void resetUniquePtr(std::unique_ptr<Person>& p)
{
    std::cout << "\n[resetUniquePtr]" << std::endl;
    p.reset(new Person("ChangedByFunction"));
}

// ============================================================
// 规则 4：T&
// 函数只关心对象，不关心 unique_ptr
// ============================================================
void usePersonByReference(Person& p)
{
    std::cout << "\n[usePersonByReference]" << std::endl;
    p.sayHello();
}


// ============================================================
// 规则 5：T*
// 函数借用对象，并且允许 nullptr
// ============================================================
void usePersonByPointer(Person* p)
{
    std::cout << "\n[usePersonByPointer]" << std::endl;
    if (p)
    {
        p->sayHello();
    }
    else
    {
        std::cout << "p is nullptr" << std::endl;
    }
}
 

int main()
{
    // 1. 创建 unique_ptr
    std::cout << "\n========== 1. create ==========" << std::endl;
    std::unique_ptr<Person> ptr1 =  std::make_unique<Person>("Alice");

    // 推荐使用 make_unique
    //
    // 也可以写：
    // std::unique_ptr<Person> ptr1(new Person("Alice"));

    // 2. 使用 operator->()
    // operator->() 返回 T*
    std::cout << "\n========== 2. operator-> ==========" << std::endl;
    ptr1->sayHello();

    // 3. 使用 operator*()
    // operator*()  返回 T&
    std::cout << "\n========== 3. operator* ==========" << std::endl;
    (*ptr1).sayHello();

    Person& ref = *ptr1;
    ref.sayHello();

    // 4. get()
    // get()：
    // - 返回内部裸指针
    // - 不转移所有权
    // - 不释放对象
    //
    // 不要这样: delete rawPtr;    
    std::cout << "\n========== 4. get() ==========" << std::endl;
    Person* rawPtr = ptr1.get();
    rawPtr->sayHello();

    // 5. unique_ptr 不能复制, 没有拷贝构造函数，因为唯一
    std::cout << "\n========== 5. no copy ==========" << std::endl;
    // 错误：
    // std::unique_ptr<Person> ptr2 = ptr1;

    // 6. std::move() 转移所有权
    std::cout << "\n========== 6. move ==========" << std::endl;
    std::unique_ptr<Person> ptr2 = std::move(ptr1);
    if (!ptr1)
    {
        std::cout << "ptr1 is nullptr after move" << std::endl;
    }
    if (ptr2)
    {
        std::cout << "ptr2 owns Alice" << std::endl;
        ptr2->sayHello();
    }

    // 7. ptr = nullptr
    // ptr2 原来管理的 Alice 会立即被释放    
    std::cout << "\n========== 7. ptr = nullptr ==========" << std::endl;
    ptr2 = nullptr;
    if (!ptr2)
    {
        std::cout << "ptr2 is nullptr" << std::endl;
    }

    // 8. reset()
    // - 释放当前对象
    // - unique_ptr 变成 nullptr    
    std::cout << "\n========== 8. reset() ==========" << std::endl;
    ptr1 = std::make_unique<Person>("Bob");
    ptr1->sayHello();
    ptr1.reset();
    if (!ptr1)
    {
        std::cout << "ptr1 is nullptr after reset()" << std::endl;
    }

    // 9. reset(new object)
    // Charlie 被释放
    // ptr1 开始管理 David    
    std::cout << "\n========== 9. reset(new object) ==========" << std::endl;
    ptr1 = std::make_unique<Person>("Charlie");
    ptr1->sayHello();
    ptr1.reset(new Person("David"));
    ptr1->sayHello();
 

    // 10. swap()  只是交换两个 unique_ptr 管理的对象
    std::cout << "\n========== 10. swap() ==========" << std::endl;
    std::unique_ptr<Person> ptr3 =
        std::make_unique<Person>("Tom");
    std::unique_ptr<Person> ptr4 =
        std::make_unique<Person>("Jerry");
    std::cout << "Before swap:" << std::endl;
    std::cout << "ptr3 -> ";
    ptr3->sayHello();
    std::cout << "ptr4 -> ";
    ptr4->sayHello();
    ptr3.swap(ptr4);
    std::cout << "After swap:" << std::endl;
    std::cout << "ptr3 -> ";
    ptr3->sayHello();
    std::cout << "ptr4 -> ";
    ptr4->sayHello();

      // 11. release()
    // - 返回内部裸指针
    // - unique_ptr 放弃所有权, 区别于 get
    // - 不 delete 对象    
    std::cout << "\n========== 11. release() ==========" << std::endl;
    std::unique_ptr<Person> ptr5 =
        std::make_unique<Person>("Mike");
    Person* releasedPtr = ptr5.release();
    if (!ptr5)
    {
        std::cout << "ptr5 is nullptr after release()" << std::endl;
    }
    releasedPtr->sayHello();

  
    // 现在没有 unique_ptr 管理这个对象，
    // 所以必须手动 delete
    delete releasedPtr;
    releasedPtr = nullptr;
  
    // ============================================================
    // 12. 函数参数：T&
    // 只使用对象，不转移所有权
    // ============================================================
    std::cout << "\n========== 12. pass T& ==========" << std::endl;
    auto ptr6 = std::make_unique<Person>("Jack");
    usePersonByReference(*ptr6);
    // ptr6 仍然拥有 Jack
    ptr6->sayHello();

  
    // ============================================================
    // 13. 函数参数：T*
    // 借用对象，不转移所有权
    // ============================================================
    std::cout << "\n========== 13. pass T* ==========" << std::endl;
    usePersonByPointer(ptr6.get());

  
    // ============================================================
    // 14. 函数参数：const unique_ptr&
    // 不转移所有权
    // ============================================================
    std::cout << "\n========== 14. pass const unique_ptr& =========="
              << std::endl;
    inspectUniquePtr(ptr6);
    // ptr6 仍然拥有 Jack
    ptr6->sayHello();
  

    // ============================================================
    // 15. 函数参数：unique_ptr&
    // 不转移所有权，但可以修改 unique_ptr
    // ============================================================
    std::cout << "\n========== 15. pass unique_ptr& =========="
              << std::endl;
    resetUniquePtr(ptr6);
    // Jack 已经被释放
    // ptr6 现在管理 ChangedByFunction
    ptr6->sayHello();
  

    // ============================================================
    // 16. 函数参数：unique_ptr 按值
    // 表示转移所有权
    // ============================================================
    std::cout << "\n========== 16. pass unique_ptr by value =========="
              << std::endl;
    auto ptr7 = std::make_unique<Person>("Rose");
    takeOwnership(std::move(ptr7));
    if (!ptr7)
    {
        std::cout << "ptr7 is nullptr after std::move()" << std::endl;
    }
  

    // 17. 临时 unique_ptr 可以直接传递，不需要再写 std::move()
    std::cout << "\n========== 17. temporary unique_ptr =========="
              << std::endl;
    takeOwnership(std::make_unique<Person>("Temporary"));

    return 0;
}
```

## Shared_ptr
```cpp
#include <iostream>
#include <memory>
#include <string>

class Person
{
public:
    explicit Person(const std::string& name) : name_(name)
    {
        std::cout << "[Create] " << name_ << std::endl;
    }

    ~Person()
    {
        std::cout << "[Destroy] " << name_ << std::endl;
    }

    void sayHello() const
    {
        std::cout << "Hello, I am " << name_ << std::endl;
    }

private:
    std::string name_;
};

int main()
{
    // ============================================================
    // 1. 创建 shared_ptr
    // ============================================================
    std::cout << "\n========== 1. create ==========" << std::endl;
    std::shared_ptr<Person> ptr1 =
        std::make_shared<Person>("Alice");


    // ============================================================
    // 2. 使用 operator->()
    // ============================================================
    std::cout << "\n========== 2. operator-> ==========" << std::endl;
    ptr1->sayHello();

    // ============================================================
    // 3. 使用 operator*()
    // ============================================================
    std::cout << "\n========== 3. operator* ==========" << std::endl;
    (*ptr1).sayHello();

    Person& ref = *ptr1;
    ref.sayHello();


    // ============================================================
    // 4. 使用 get()
    // ============================================================
    std::cout << "\n========== 4. get() ==========" << std::endl;
    Person* rawPtr = ptr1.get();
    rawPtr->sayHello();

    // get()：
    // - 返回内部裸指针
    // - 不改变 shared_ptr 的所有权
    //
    // 不要手动：
    // delete rawPtr;


    // ============================================================
    // 5. shared_ptr 可以复制
    // ============================================================
    std::cout << "\n========== 5. copy ==========" << std::endl;
    std::shared_ptr<Person> ptr2 = ptr1;
    // 现在 ptr1 和 ptr2 共同拥有 Alice
    ptr1->sayHello();
    ptr2->sayHello();


    // ============================================================
    // 6. 查看引用计数 use_count()
    // ============================================================
    std::cout << "\n========== 6. use_count() ==========" << std::endl;
    std::cout << "ptr1 use_count = "
              << ptr1.use_count() << std::endl;
    std::cout << "ptr2 use_count = "
              << ptr2.use_count() << std::endl;
    // 此时应该都是 2


    // ============================================================
    // 8. ptr3 = nullptr
    // ============================================================
    std::cout << "\n========== 8. ptr3 = nullptr =========="
              << std::endl;
    ptr3 = nullptr;
    std::cout << "use_count = "
              << ptr1.use_count() << std::endl;
    // ptr3 不再拥有 Alice
    // 引用计数从 3 变成 2


    // ============================================================
    // 9. reset()
    // ============================================================
    std::cout << "\n========== 9. reset() =========="
              << std::endl;
    ptr2.reset();
    std::cout << "use_count = "
              << ptr1.use_count() << std::endl;
    // ptr2 放弃所有权
    // 此时只剩 ptr1



    // ============================================================
    // 10. 最后一个 shared_ptr reset()
    // ============================================================
    std::cout << "\n========== 10. last reset =========="
              << std::endl;
    ptr1.reset();
    // 引用计数变成 0
    // Alice 在这里自动析构


    // ============================================================
    // 11. 判断 shared_ptr 是否为空
    // ============================================================
    if (!ptr1)
    {
        std::cout << "ptr1 is nullptr" << std::endl;
    }


    std::cout << "\n========== End of main =========="
              << std::endl;

    return 0;
}
```

## Forward

主要用途：保留参数的左值或者右值属性。

```
#include <iostream>
#include <utility>

void process(int& value)
{
    std::cout << "lvalue: " << value << '\n';
}

void process(int&& value)
{
    std::cout << "rvalue: " << value << '\n';
}

template<typename T>
void wrapper(T&& value)
{
    // value 可能是左值或右值，仅在 模板中可以这么写
    // 保留了 value 的左值或右值属性，传递给process
    process(std::forward<T>(value));
}

int main()
{
    int a = 10;

    wrapper(a);
    wrapper(20);

    return 0;
}
```


## Variant

```
#include <iostream>
#include <string>
#include <variant>

int main()
{
    using Value = std::variant<int, std::string, double>;

    // ============================================================
    // 1. 创建
    // ============================================================
    Value v1 = 100;
    std::cout << "index = " << v1.index() << std::endl;


    // ============================================================
    // 2. std::get<T>()
    // ============================================================
    std::cout << "\n--- get<T>() ---" << std::endl;
    int n = std::get<int>(v1);
    std::cout << "value = " << n << std::endl;


    // ============================================================
    // 3. 改变当前保存的类型
    // ============================================================
    std::cout << "\n--- change type ---" << std::endl;
    v1 = std::string("hello");
    std::cout << "index = " << v1.index() << std::endl;


    // ============================================================
    // 4. holds_alternative<T>()
    // ============================================================
    std::cout << "\n--- holds_alternative ---" << std::endl;
    if (std::holds_alternative<std::string>(v1))
    {
        std::cout << "v1 contains string" << std::endl;
        std::cout << std::get<std::string>(v1) << std::endl;
    }


    // ============================================================
    // 5. get_if<T>()
    // ============================================================
    std::cout << "\n--- get_if<T>() ---" << std::endl;
    if (auto p = std::get_if<std::string>(&v1))
    {
        std::cout << "string = " << *p << std::endl;
    }
    else if (auto p = std::get_if<int>(&v1))
    {
        std::cout << "int = " << *p << std::endl;
    }
    else
    {
        std::cout << "double = " << *p << std::endl;
    }


    // ============================================================
    // 6. std::visit()
    // ============================================================
    std::cout << "\n--- visit ---" << std::endl;
    std::visit(
        [](const auto& value)
        {
            std::cout << "actual value = "
                      << value
                      << std::endl;
        },
        v1);


    // ============================================================
    // 7. 再切换成 double
    // ============================================================
    std::cout << "\n--- double ---" << std::endl;
    v1 = 3.14159;
    std::visit(
        [](const auto& value)
        {
            std::cout << "actual value = "
                      << value
                      << std::endl;
        },
        v1);

    return 0;
}
```


## Thread

```
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

void printMessage(
    int id,
    const std::string& message)
{
    std::cout
        << "thread id = "
        << std::this_thread::get_id()
        << ", worker = "
        << id
        << ", message = "
        << message
        << '\n';
}

void changeValue(int& value)
{
    value = 100;
}

int main()
{
    std::cout
        << "main thread id = "
        << std::this_thread::get_id()
        << '\n';

    // 普通函数
    std::thread t1(
        printMessage,
        1,
        "hello");

    // Lambda
    std::thread t2([]() {
        std::cout
            << "lambda thread id = "
            << std::this_thread::get_id()
            << '\n';

        std::this_thread::sleep_for(
            std::chrono::milliseconds(500));
    });

    // 引用参数
    int value = 10;

    std::thread t3(
        changeValue,
        std::ref(value));

    if (t1.joinable())
    {
        t1.join();
    }

    if (t2.joinable())
    {
        t2.join();
    }

    if (t3.joinable())
    {
        t3.join();
    }

    std::cout
        << "value = "
        << value
        << '\n';

    std::cout << "main finished\n";

    return 0;
}
```


