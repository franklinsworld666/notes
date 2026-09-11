
## 题目

https://leetcode.cn/problems/unique-binary-search-trees-ii/description/

## tag 
二叉树
减治
递归

## soulution1
选择 i 作为根节点，这时所有的二叉搜索树，就取决于由[1,i-1]组成的全部左子树 * [i+1, n]组成的全部右子树；

时间复杂度: 不懂
空间复杂度: 不懂




## code 
```python3
class Solution:
    def generateTrees(self, n: int) -> List[Optional[TreeNode]]:
        
        def dfs(l:int, r:int) -> List[Optional[TreeNode]]:
            cur = []
            if l > r:
                cur.append(None)
                return cur
            for i in range(l, r+1):
                for j in dfs(l,i-1):
                    for k in dfs(i+1, r):
                        node = TreeNode(i, None, None)
                        node.left = j
                        node.right = k
                        cur.append(node)
            return cur
        
        return dfs(1, n)
```


    

