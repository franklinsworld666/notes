
## 题目

https://leetcode.cn/problems/binary-tree-maximum-path-sum/description/?envType=problem-list-v2&envId=tree

## tag 
二叉树
减治
DFS



## soulution1

+ 节点A的 max-path-sum = max(左孩子的max-path-sum , 右孩子的max-path-sum, 经过节点A的max-path-sum)
+ 经过节点A的max-path-sum, 或者是从A出发只经过左子树的路径sum，或者是从A出发只经过右子树的路径sum，或者是从左子树中的某个叶子节点出发经过节点A到达右子树的叶子节点;
+ 为了减少重复计算，针对每个节点X可以用字典保存从X出发只经过左子树的路径sum， 从X出发只经过右子树的路径sum；

time:O(n), 每一个节点X的计算

space:O(n)

## solution2

+ 定义节点A的最大贡献值：从A从发的一条路径，并且该路径上的节点值之和最大；
+ 通过节点A的最大路径和：取决于 A , A.left的最大贡献值，A.right的最大贡献值；
+ 记录每个节点的最大路径和，保存其中的最大值
+ 比方法一的优化之处：直接在计算最大贡献值时，计算了最大路径和，减少了递归次数，简化代码


## code 
```python3
class Solution:
	def maxRootPathSum(self, root:Optional[TreeNode]) -> int:
		if not root:
			return -30000000

		if not root.left and not root.right:
			self.mydict[id(root)] = root.val
			return root.val

		lm = -30000000
		rm = -30000000
		p1 = root.val
		if root.left:
			lm = self.maxRootPathSum(root.left)
		if root.right:
			rm = self.maxRootPathSum(root.right)
		
		self.mydict[id(root)] = p1 + max(0, lm, rm)
		return p1 + max(0, lm, rm)

	def maxChildPathSum(self, root:Optional[TreeNode]) -> int:
		if not root:
			return 0
		
		lm = -30000000
		rm = -30000000
		if root.left:
			lm = self.maxChildPathSum(root.left)
		if root.right:
			rm = self.maxChildPathSum(root.right)		
		
		if root.left:
			pl = self.mydict[id(root.left)]
		else:
			pl = -30000000
		if root.right:
			pr = self.mydict[id(root.right)]
		else:
			pr = -30000000
		p = root.val + max(0, pl, pr, pl+pr)

		return max(lm, rm, p)	
		

	def maxPathSum(self, root: Optional[TreeNode]) -> int:
		if not root:
			return 0
		
		if not hasattr(self, 'mydict'):
			self.mydict = {}
		self.maxRootPathSum(root)

		lm = -30000000
		rm = -30000000
		if root.left:
			lm = self.maxChildPathSum(root.left)
		if root.right:
			rm = self.maxChildPathSum(root.right)		
		
		if root.left:
			pl = self.mydict[id(root.left)]
		else:
			pl = -30000000
		if root.right:
			pr = self.mydict[id(root.right)]
		else:
			pr = -30000000
		p = root.val + max(0, pl, pr, pl+pr)

		return max(lm, rm, p)	
				
```

```python3
class Solution:
	def maxRootPathSum(self, root:Optional[TreeNode]) -> int:
		if not root.left and not root.right:
			self.ans = max(self.ans, root.val)
			return root.val

		lm = -30000000
		rm = -30000000
		p1 = root.val
		if root.left:
			lm = self.maxRootPathSum(root.left)
		if root.right:
			rm = self.maxRootPathSum(root.right)
		
		# 通过该节点的最大路径和
		priceMax = root.val + max(0, lm) + max(0, rm)
		self.ans = max(self.ans, priceMax)

		return p1 + max(0, lm, rm)

	def maxPathSum(self, root: Optional[TreeNode]) -> int:
		if not root:
			return 0
		
		if not hasattr(self, 'ans'):
			self.ans = -30000000

		self.maxRootPathSum(root)
		return self.ans
				
```