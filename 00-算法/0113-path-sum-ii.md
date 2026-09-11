
## 题目

https://leetcode.cn/problems/path-sum-ii/description/?envType=problem-list-v2&envId=tree

## tag 
二叉树
BFS

## soulution1

+ 对树进行深度优先遍历，找出每一条路径，再计算出符合条件的路径

time:O(n * logn), 时间复杂度等于 添加进入列表的节点的数量  

space:O(nlogn)，路径的条数等于叶子节点的数量O(n)，每条路径的长度为 O(logn)

## soulution2

+ 对树进行深度优先遍历，遍历每一条路径，记录符合条件的路径

time:O(n * logn), 时间复杂度等于 添加进入列表的节点的数量, 平均O(nlogn), 最坏O(N * N)

space:O(logn)，每条路径的长度为 O(logn)，最坏情况O(N)


## code 
```python3
class Solution:
    def pathSum(self, root: TreeNode, s: int) -> List[List[int]]:
        if root is None:
            return []
        paths = []
        def dfs(root, nodes):
            if root is None:
                return
            nodes.append(root.val)
            if root.left is None and root.right is None:
                paths.append(nodes)
                return
            dfs(root.left, nodes[:])
           
            dfs(root.right, nodes[:])
           
        dfs(root, [])
        return [item for item in paths if sum(item) == s]
		
```

```python3
class Solution:
    def pathSum(self, root: TreeNode, s: int) -> List[List[int]]:
        ans = []
        path = []

        def dfs(root, target):
            if not root:
                return 
            path.append(root.val)
            if not root.left and not root.right and root.val == target:
                ans.append(path[:])
            
            if root.left:
                dfs(root.left, target - root.val)
            
            if root.right:
                dfs(root.right, target - root.val)
            
            path.pop()
        
        dfs(root, s)
        return ans


```