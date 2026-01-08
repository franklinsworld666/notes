## 题目
https://leetcode.cn/problems/shortest-distance-to-a-character/description/

## tag 
数组

## 思路1
1. 遍历一次字符串，找出所有字符c的位置，存入一个数组cqueue，记得加上左边界-100000，右边界100000；
2. 再次遍历字符串，对于位置 i 的字符x，找出 cqueue中第一个大于等于 i 的值，这个值就是距离字符x最近的右边界，前一个值必定是字符x的左边界；取其中的最小距离；

时间复杂度：O(2N)
空间复杂度：O(N)

## 思路2
首先，设置一个左边界-100000，从左向右遍历，计算出每一个字符与其最近的左边界的值；每次遇到字符c，更新左边界；
其次，设置一个右边界100000，从右向左遍历，计算出min(每一个字符与其最近的右边界的值, 与其左边界的值)；每次遇到字符c，更新右边界；

时间复杂度：O(2N)
空间复杂度: O(1)

## 代码1
```python3
class Solution:
    def shortestToChar(self, s: str, c: str) -> List[int]:
        cqueue = [-100000]
        for i in range(len(s)):
            if s[i] == c:
                cqueue.append(i)
        cqueue.append(100000)
        ans = [0] * len(s)
        index = 0
        for i in range(len(s)):
            while cqueue[index] < i:
                index += 1
            num = min(i - cqueue[index-1], cqueue[index]-i)
            ans[i] = num
        return ans

```

## 代码2
```python3
class Solution:
    def shortestToChar(self, s: str, c: str) -> List[int]:
        pre = -100000
        ans = [0] * len(s)
        for i in range(len(s)):
            if s[i] == c:
                pre = i
            ans[i] = i - pre
        pre = 100000
        for i in range(len(s)-1, -1, -1):
            if s[i] == c:
                pre = i
            ans[i] = min(ans[i], pre - i)
        return ans
```