
## 题目

https://leetcode.cn/problems/implement-queue-using-stacks/description/

## tag
链表
栈
队列

## solution
用双端队列来实现栈。 

用栈A 来保存队列push 进来的元素，用栈B来支持pop和peek操作。如果栈B为空，那么把栈A全部出栈并插入栈B，此时栈B中的元素出栈顺序刚好与队列一致，可以用来进行队列pop和peek操作；如果栈B不为空，则直接对栈B进行 pop和peek操作；

时间复杂度：
+ push: O(1)， 同双端队列的push操作
+ empty: O(1)
+ pop/peek: 最好O(1), 最坏O(N), 平均来讲O(1); 假设在栈A中存在X个元素时，进行pop/peek操作，那么要进行X次出栈入栈操作，也意味着接下来的 X-1 次pop/peek 操作只要常数时间，所以平均来讲时间复杂度是O(1)

空间复杂度: O(N)

## code 

```python3
class MyQueue:

    def __init__(self):
        self.stack1 = MyStack()
        self.stack2 = MyStack()

    def push(self, x: int) -> None:
        self.stack2.push(x)

    def pop(self) -> int:
        if not self.stack1.empty():
            return self.stack1.pop()
        else:
            while not self.stack2.empty():
                self.stack1.push(self.stack2.pop())
            return self.stack1.pop()

    def peek(self) -> int:
        if not self.stack1.empty():
            return self.stack1.peek()
        else:
            while not self.stack2.empty():
                self.stack1.push(self.stack2.pop())
            return self.stack1.peek()

    def empty(self) -> bool:
        return self.stack1.empty() and self.stack2.empty()

class MyStack:
    def __init__(self):
        self.stack = deque()

    def push(self,x):
        self.stack.append(x)
    
    def pop(self) -> int:
        if not self.empty():
            return self.stack.pop()
        else:
            return None
    
    def peek(self) -> bool:
        if not self.empty():
            return self.stack[-1]
        else:
            return None
    
    def empty(self) -> bool:
        return len(self.stack) == 0

```

