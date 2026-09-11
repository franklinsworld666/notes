## 题目
https://leetcode.cn/problems/unique-substrings-in-wraparound-string/description/

## tag
前缀和
递推

## 思路
从前往后，计算出以 i 位结尾的子数组个数；这个子数组需要满足连续性；
然后，记录以不同字符结尾的子数组的最大长度；
最后，把 a-z 结尾的子数组的长度相加；

时间复杂度：O(N)
空间复杂度：O(1)

不变性：计算出以 i 位结尾的子数组个数
单调性：从前往后遍历，取以x结尾的子数组的最大值；


## 代码
```python3
import collections

class Solution:
	def findSubstringInWraproundString(self, s: str) -> int:
		num = 1
		len_map = collections.defaultdict(lambda:0)
		len_map[s[0]] = 1
		for i in range(1, len(s)):
			if ord(s[i]) - ord('a') == (ord(s[i-1]) + 1 - ord('a')) % 26:
				num += 1
			else:
				num = 1
			if num > len_map[s[i]]:
				len_map[s[i]] = num
		return sum(len_map.values())
```