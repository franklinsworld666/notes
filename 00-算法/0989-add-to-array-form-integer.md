## 题目
https://leetcode.cn/problems/add-to-array-form-of-integer/

## tag 
数组

## 思路
首先，把整数转化为列表；
然后，从后往前相加；
注意，进位

时间复杂度: O(数字长度)
空间复杂度: O(数字长度)

## 代码
```python3
from typing import List

class Solution:
	def addToArrayForm(self, num:List[int], k:int) -> List[int]:
		b = list(str(k))
		i,j = len(num)-1, len(b)-1
		add = 0
		while i >= 0 and j >= 0:
			c = (ord(b[j]) - ord('0') + num[i] + add) % 10
			add = (1 if ord(b[j]) - ord('0') + num[i] + add > 9 else 0)
			num[i] = c 
			i -= 1
			j -= 1
		if i < 0 and j < 0 and add > 0:
			num.insert(0,1)
		elif j < 0:
			while i >= 0 and add > 0:
				c = (num[i] + add) % 10
				add = (1 if num[i] + add > 9 else 0)
				num[i] = c 
				i -= 1
			if add > 0:
				num.insert(0,1)
		elif i < 0:
			while j >= 0:
				c = (ord(b[j]) - ord('0') + add)% 10
				add = (1 if ord(b[j]) - ord('0') + add > 9 else 0)
				num.insert(0, c)
				j -= 1
			if add > 0:
				num.insert(0,1)
		return num
```