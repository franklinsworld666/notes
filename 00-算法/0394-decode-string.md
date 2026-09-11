## 题目
https://leetcode.cn/problems/decode-string/


## tag
栈

## 思路1
转换成一个普通的中缀表达式，就是说在字符串中把缺失的 * + 给他补上；
然后这就成为了一个普通的中缀表达式计算

时间复杂度:O(N)

## 思路2
分层计算，以 "[" 来分层，以 "]" 来结束该层的计算。
每次遇到 "[" ，保存上一层的字符串 和 数字 到栈中，遇到"]" 把数字出栈计算出本层的结果，并和上层的字符串结果合并。

时间复杂度:O(N)
空间复杂度:O(N)

单调性：逐一遍历字符串直到结束
不变性：没遇到一个 右括号，消掉一层；



## 代码
```python3
class Solution:
    def decodeString(self, s: str) -> str:
        s1 = []
        s2 = []
        num = 0
        res = ""
        for c in s:
            if c.isdigit():
                num = num * 10 + int(c)
            elif c == "[":
                s2.append(res)
                s1.append(num)
                num,res = 0, ""
            elif c == "]":
                res = s2.pop() + s1.pop() * res
            else:
                res += c
        return res
```