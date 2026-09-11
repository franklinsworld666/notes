## 题目
https://leetcode.cn/problems/find-all-duplicates-in-an-array/description/

## tag 
数组
hash

## 思路1
由于数组长度为 n，元素取值为 1-n，每个元素出现一次或两次，那么我们只要尽可能把每个元素放到正确的位置，剩下的就是重复元素。
具体来讲，就是尽可能把 i，放到(i-1)位置上。

时间复杂度: O(N)
空间复杂度: O(1)

不变性：随着从左到右遍历，左边的元素，或者是已经就位的，或者是无法就位的重复元素

单调性：随着遍历，未处理的元素单调减少；

## 思路2
用一个数组ans, 大小为 N，存放每个元素的个数；

时间复杂度: O(N),比思路1效率更高
空间复杂度: O(N)


## 代码
```python3
class Solution:
    def findDuplicates(self, nums: List[int]) -> List[int]:
        ans = []
        len_n = len(nums)
        i = 0
        while i < len_n:
            if nums[i] > 0 and nums[i] != i+1:
                tmp = nums[i]
                if tmp != nums[tmp-1]:
                    nums[i] = nums[tmp-1]
                    nums[tmp-1] = tmp
                else:
                    ans.append(tmp)
                    nums[i] = -1
                    i += 1
            else:
                i += 1
        i = 0
        while i < len_n:
            if nums[i] != i+1:
                ans.append(nums[i])
            i += 1
        return ans

```