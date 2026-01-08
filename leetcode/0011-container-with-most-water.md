
## 题目

https://leetcode.cn/problems/container-with-most-water/description/

## tag 
数组
减治

## solution1
2层循环遍历整个数组，计算每一个矩阵的面积，找出其中的最大值。

时间：O(n*n)
空间：O(1)

## solution2

在循环遍历的基础上，进行减枝。循序遍历过程中的某些矩阵，其实根本不用考虑。
以如下数组为例：
0    1    2    3   4   5   6   7   8
1    8    6    2   5   4   8   3   7

最大值取值范围：
[0,8]，
[1,8],
[1,7]肯定不是，7不可能是右边界，否则把右边界换成8，会获得更大的矩形，
[1,6],
2,3,4,5都不可能是左边界了


如果我们把可能的左边界用线段连接起来，会发现它是一个从左往右、从低严格到高的折线；
右边界，是从右往左、从低到严格到高的折线；
中间部分，不可能成为左边界或右边界，因为有更大的选择。

假设左边界分别为 l1,l2,...ll, 右边界分别为r1,r2...rr,
可能的选择[l1, r1], ... [lx, r1](lx 为第一个大于r1的)
接下来 [lx, r1],[lx,r2]... 以此类推
这里面也存在减枝的操作，并不需要 l * r 次最大面积计算；

时间:O(n)
空间:O(1)


## code 

```python3
class Solution:
    def maxArea(self, height: List[int]) -> int:
        lo = 0
        hlo = height[lo]
        hi = len(height) - 1
        hhi = height[hi]
        ret = 0
        while lo < hi:
            if hlo < hhi:
                ret = max(ret, (hi-lo)*hlo)
                while lo < hi and height[lo] <= hlo:
                    lo += 1
                hlo = height[lo]
            else:
                ret = max(ret, (hi-lo)*hhi)
                while lo < hi and height[hi] <= hhi:
                    hi -= 1
                hhi = height[hi]
        return ret

```