
## 题目

https://leetcode.cn/problems/binary-tree-inorder-traversal/description/

## tag 
二叉树


## soulution1

递归

时间:O(N)

空间:O(logN)

## solution2

+ 先把根节点入栈，颜色为白色；
+ 把栈中节点出栈，如果节点为灰色，直接访问；
+ 如果节点为白色，先把右孩子入栈（白色），再把自身入栈（灰色），最后把做孩子入栈（白色）


time:O(N)

space:O(logN)

## code 
```python3

# class TreeNode:
#     def __init__(self, val=0, left=None, right=None):
#         self.val = val
#         self.left = left
#         self.right = right

class Solution:
    def inorderTraversal(self, root: Optional[TreeNode]) -> List[int]:
        ans = []
        stack = []
        if root:
            stack.append((root, 0))
        while stack:
            item = stack.pop()
            if item[1] == 0:
                if item[0].right:
                    stack.append((item[0].right, 0))
                stack.append((item[0], 1))
                if item[0].left:
                    stack.append((item[0].left, 0))
            elif item[1] == 1:
                ans.append(item[0].val)
        return ans
```
