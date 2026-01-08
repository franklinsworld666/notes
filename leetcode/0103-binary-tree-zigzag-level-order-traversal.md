
## 题目

https://leetcode.cn/problems/binary-tree-zigzag-level-order-traversal/description/?envType=problem-list-v2&envId=tree

## tag 
二叉树
层次遍历

## soulution1

+ 对树进行层次遍历
+ 针对每层遍历时，根据标志来决定遍历顺序

time:O(n)

space:O(n)



## code 
```python3
class Solution:
	def zigzagLevelOrder(self, root: Optional[TreeNode]) -> List[List[int]]:
		ans = []
		que = collections.deque()

		if not root:
			return ans

		que.append(root)
		dir = True

		while que:
			level_size = len(que)
			ret = []
			if dir:
				for node in que:
					ret.append(node.val)
			else:
				for node in reversed(que):
					ret.append(node.val)
			ans.append(ret)
			dir = not dir

			for _ in range(level_size):
				node = que.popleft()
				if node.left:
					que.append(node.left)
				if node.right:
					que.append(node.right)
		
		return ans
```