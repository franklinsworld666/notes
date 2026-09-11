
## 题目

https://leetcode.cn/problems/populating-next-right-pointers-in-each-node-ii/?envType=problem-list-v2&envId=tree

## tag 
二叉树
层次遍历

## soulution1

+ 对树进行层次遍历
+ 针对每层遍历时，处理实际业务

time:O(n)

space:O(n)



## code 
```python3
class Solution:
	def connect(self, root: 'Node') -> 'Node':
		if not root:
			return None
		
		q = collections.deque()
		q.append(root)

		while q:
			qlen = len(q)

			pre = q.popleft()
			pre.next = None
			if pre.left:
				q.append(pre.left)
			if pre.right:
				q.append(pre.right)
			
			for i in range(1, qlen):
				post = q.popleft()
				pre.next = post
				pre = post 

				if pre.left:
					q.append(pre.left)
				if pre.right:
					q.append(pre.right)
			
			pre.next = None
		
		return root
				
```