## 题目
https://leetcode.cn/problems/spiral-matrix-ii/description/

## tag 
数组

## solution
首先，定义一个二维矩阵，初值全部是0，从左上角开始，不断向右走，走到尽头向下，向左，向上；
时间复杂度: O(N*N)
空间复杂度: O(N*N)

## 代码
```python3
class Solution:
    def generateMatrix(self, n: int) -> List[List[int]]:
        array_2d = [[0] * n for _ in range(n)]
        dir = [[0,1],[1,0],[0,-1],[-1,0]]
        index = 0
        s,x,y=1,0,0
        while True:
            if x < 0 or x >= n or y < 0 or y >= n or array_2d[x][y] != 0:
                break
            array_2d[x][y] = s
            s += 1
            n_x = dir[index][0]
            n_y = dir[index][1]
            if (x + n_x >= 0 and x + n_x < n
                and y + n_y >= 0 and y + n_y < n
                and array_2d[x+n_x][y+n_y] == 0):
                x += n_x
                y += n_y
            else:
                index = (index + 1) % 4
                x = x + dir[index][0]
                y = y + dir[index][1]
        return array_2d
```