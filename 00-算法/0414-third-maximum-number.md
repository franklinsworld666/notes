
## 题目
https://leetcode.cn/problems/third-maximum-number/

## tag 
数组

## solution 

首先，定义三个变量，初始值为一个比元素可能的取值范围更小的数；
然后，从头开始遍历，比较；
最后，如果第三大的数存在，返回；不存在，就返回最大的数；

时间复杂度：O(N);
空间复杂度：O(1);

## code 
```python3
class Solution:
    def thirdMax(self, nums: List[int]) -> int:
        first = -2**32
        second = -2**32
        third = -2**32
        for index in range(0, len(nums)):
            if nums[index] > first:
                third = second
                second = first
                first = nums[index]
            elif nums[index] != first:
                if nums[index] > second:
                    third = second
                    second = nums[index]
                elif nums[index] != second:
                    if nums[index] > third:
                        third = nums[index]
        if third != -2**32:
            return third
        else:
            return first
```