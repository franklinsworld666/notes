## 题目
https://leetcode.cn/problems/number-of-subarrays-with-bounded-maximum/

## tag
递推
前缀和

## 思路
最大值为 right 的子数组数量 - 最大值为 left-1 的子数组数量；

时间复杂度: O(n)
空间复杂度: O(1)


## 代码
```python3
from typing import List

class Solution:
	def atmost(self, nums:List[int], maxn:int) -> int:
		ans = 0
		num = 0
		for n in nums:
			if n > maxn:
				num = 0
			else:
				num += 1
			ans += num
		return ans		

	def numSubarrayBoundedMax(self, nums: List[int], left: int, right: int) -> int:
		return self.atmost(nums, right) - self.atmost(nums, left-1)
```