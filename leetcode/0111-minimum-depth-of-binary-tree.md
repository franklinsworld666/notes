
## 题目

https://leetcode.cn/problems/minimum-depth-of-binary-tree/description/?envType=problem-list-v2&envId=tree

## tag 
二叉树
BFS

## soulution1

+ 对树进行层次遍历
+ 针对每层遍历时，如果遇到叶子节点，终止

time:O(n)

space:O(n)



## code 
```python3
class Solution:
	def minDepth(self, root: Optional[TreeNode]) -> int:
		que = collections.deque()

		if not root:
			return 0

		que.append(root)
		res = 0

		while que:
			res = res + 1
			level_size = len(que)

			for _ in range(level_size):
				node = que.popleft()
				if node.left:
					que.append(node.left)
				if node.right:
					que.append(node.right)
				
				if not node.left and not node.right:
					return res
```