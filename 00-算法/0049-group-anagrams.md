## 题目
https://leetcode.cn/problems/group-anagrams/

## tag 
hash

## 思路
把 每一个字符串重新排序，作为关键值放入 字典/hash表，它的值为 ans 中的下标；
对于一个新的字符串，如果可以在字典中找到，那么就必定属于同一个 list；
如果没有在字典中找到，那么就在 ans 中新增一个list元素，并在字典中记录下标；


时间复杂度: O(N * klogk)
空间复杂度: O(N * k)


## 代码
```python3

from typing import List

class Solution:
	def groupAnagrams(self, strs: List[str]) -> List[List[str]]:
		ans = []
		hash_dict = {}
		
		for s in strs:
			s1 = ''.join(sorted(s))
			if s1 in hash_dict:
				ans[hash_dict[s1]].append(s)
			else:
				hash_dict[s1] = len(ans)
				ans.append([s])
		return ans
```