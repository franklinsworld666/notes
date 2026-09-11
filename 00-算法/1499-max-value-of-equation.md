## 题目
https://leetcode.cn/problems/max-value-of-equation/description/

## tag
单调队列
队列

## 思路1

针对任意2个点，进行计算，找出满足条件的最大值

时间复杂度:O(N * N)
空间复杂度:O(1)

## 思路2

yi + yj + | xj - xi | 等价于 xj + yj + (yi - xi)

所以针对点 j， 只要找出前面k距离内 (yi - xi) 的最大的点， 这样的2个点构成了可能的一个解

找出所有可能解中的最大值即可

时间复杂度:O(N)
空间复杂度:O(k)

## 代码
```python3
class Solution:
	def findMaxValueOfEquation(self, points:List[List[int]], k:int) -> int:
		ans = None
		q = deque()
		for x2, y2 in points:
			# 队头所有超出范围的，出队
			# 如果非空，记录可能的ans
			# 入队 (所有更小的出队)
			while q :
				x1,y1 = q[0]
				if x2 - x1 > k:
					q.popleft()
				else:
					break
			
			if q:
				x1,y1 = q[0]
				val = x2 + y2 + y1 - x1
				if ans is None or ans < val:
					ans = val
			
			while q:
				x1,y1 = q[-1]
				if y1 - x1 <= y2 - x2:
					q.pop()
				else:
					break
			
			q.append([x2,y2])
		return ans
```