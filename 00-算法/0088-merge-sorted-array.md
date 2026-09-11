

## 题目

https://leetcode.cn/problems/merge-sorted-array/

## tag 
数组

## solution

合并2个有序数组，但是和一般的合并不同的是，它采用的是从大到小合并，即先把最大的数放到数组最后边，这样节省了操作。

时间复杂度:O(m+n)
空间复杂度:O(m+n)


## code 
```python3
class Solution:
    def merge(self, nums1: List[int], m: int, nums2: List[int], n: int) -> None:
        """
        Do not return anything, modify nums1 in-place instead.
		the size of nums1 is equal to m+n
        """
        p1 = m-1
        p2 = n-1
        i = m+n-1
        while p1 >=0 and p2 >= 0:
            if nums1[p1] >= nums2[p2]:
                nums1[i] = nums1[p1]
                p1 -= 1
            else:
                nums1[i] = nums2[p2]
                p2 -= 1
            i -= 1
        while p2 >= 0:
            nums1[i] = nums2[p2]
            p2 -= 1
            i -= 1

```