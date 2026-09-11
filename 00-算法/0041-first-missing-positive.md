
## 题目
https://leetcode.cn/problems/first-missing-positive/description/

## tag 
数组
哈希表


## solution1
用一个 hash表 存放数组中的全部元素， 然后从 1 - len(nums)+1 进行检查，看哪一个元素不在里面；

时间复杂度:O(N)
空间复杂度:O(N)

## solution2
首先，从前往后遍历，对于 [0, len(nums)] 的数，通过交换调整到对应的位置；对于超出范围的数，丢弃；
最后，再遍历一次，检查哪一个位置上的正数缺失；

时间复杂度:O(2N)
空间复杂度:O(1)


## 代码
```python3
class Solution:
	def firstMissingPositive(self, nums: List[int]) -> int:
		index = 0
		lenn = len(nums)
		while index < lenn:
			post = nums[index]
			if post <= 0 or post > len(nums):
				index += 1
			else:
				if post == nums[post-1]:
					index += 1
				else:
					nums[index] = nums[post-1]
					nums[post-1] = post
		for index in range(lenn):
			if nums[index] != index+1:
				return index+1
		return lenn + 1
```



		