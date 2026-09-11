
# 复制构造函数、赋值函数
```cpp
string::string(const string& rhs)
{
    int len = strlen(rhs.m_str);
    m_str = new char[len+1];
    strncpy(m_str, rhs.m_str, len);
    m_str[len] = '\0';
}

string& string::operator=(const string& rhs)
{
    if (*this == rhs) return *this;
    string temp(rhs);
    swap(*this, temp);
    return *this;

    /*
    if (*this == rhs) return *this;
    char* temp = m_str;
    int len = strlen(rhs.m_str);
    m_str = new char[len+1];
    strncpy(m_str, rhs.m_str, len);
    m_str[len] = '\0';
    delete [] temp;
    return *this;
    */
}
```

# 单例模式
```CPP
class CSingleton {
    public:
        static CSingleton* instance();
    private:
        CSingleton();
        ~CSingleton();
        CSingleton(const CSingleton&);
        CSingleton& operator=(const CSingleton&);
};
CSingleton* CSingleton::instance()
{
    static CSingleton inst;
    return &inst;
}
```

# 问题
1. pure virtual有什么作用？
    把这个类作为抽象类，不允许实例化，只能继承，是作为接口来使用的。在派生类中，需要实现这个virtual函数

2. 为什么构造和析构函数中不能调用virtual函数
+ 构造的时候，先是基类构造函数再是子类构造函数，在基类构造函数中调用virtual时，派生类还没有构造出来
+ 在析构函数中，调用virtual，很可能有些成员变量已经不存在了

3. const成员函数中不能调用non-const成员函数
因为non-const可能修改成员变量

4. CPP默认函数
+ 默认构造、默认析构
+ 默认复制构造、operator=函数

5. 四种不同的转型，何时用
+ static_cast , 用在本来就可以隐式转换的地方。好处：方便编译器发现错误，容易发现哪些地方发生了转型
+ reinterpret_cast, 不能隐式转换的地方
+ dynamic_cast, 我只有基类指针，却想访问派生类方法。最好用virtual函数来做这件事，不要用这种方法。
+ const_cast

6. explict作用
不允许隐式转换。

7. inline适用什么函数
小的，又频繁调用的函数。

8. 覆盖、重载、虚函数
+ 覆盖：又叫隐藏，子类中又一个函数和父类函数名相同，于是父类的函数看不见了
+ 重载：同一个类中，函数名相同，参数不同的两个函数
+ 虚函数：父类和子类中


8. 删除元素
+ vector: c.erase(remove(v.begin(), v.end(), 1963), v.end())
+ list:c.erase(1963)

9. Find查找文件
+ find: 搜寻硬盘查找文件，`find <指定目录> <指定条件> <指定动作>`
```
find ./ -name "7*" -ls
find ./ -name "7*" -exec ls -l {} \;
```

10. grep 查找文件内容
+ `grep [OPTIONS] PATTERN [FILE...]`
```
grep -rn "AlarmLocal" ./  # 在目录下所有文件中递归查找AlarmLocal的文件
```

11. Makefile 中=  :=  ?= 区别
+ = 赋值，要在全部展开后才能确定
+ ?= 如果没有赋值，就赋予该值
+ := 与当前位置有关


# MYSQL
```MYSQL
CREATE TABLE interest
(
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    interest VARCHAR(32) NOT NULL
);

INSERT INTO interest
(id, interest)
VALUES('1', 3.89);

ALTER TABLE interest
ADD COLUMN contactId INT NOT NULL PRIMARY KEY;

ALTER TABLE interest
DROP COLUMN contactId;


UPDATE interest SET interest = 0.37 WHERE id = 2;

UPDATE interest SET interest =
CASE
    WHEN id = 2 THEN 3.7
    WHEN id = 3 THEN 4
    ELSE 4.5
END;

DELETE FROM interest WHERE id = 2;
```

# Network Programming
+ 服务端：socket, bind, listen, accept, read/write, close
+ 客户端：socket, connect, read/write, close
