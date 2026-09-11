## 题目

https://leetcode.cn/problems/construct-binary-tree-from-preorder-and-inorder-traversal/description/?envType=problem-list-v2&envId=tree

## tag

二叉树
前序遍历
中序遍历

## soulution1

+ 利用前序遍历，找到根节点；
+ 利用中序和根节点，把树拆分为左子树和右子树；
+ 分别对左子树和右子树进行上面的操作；

time:O(nlogn), 在中序遍历中查找节点的操作，最多logn 层，每层加起来不会超过O(n)

space:O(n), 需要n的空间来存放节点

## soulution2
可以先建立一个hash表，优化节点的查找，建立hash表的过程O(n), 只是增加了空间消耗

time:O(n)

space:O(n)

## code

```python3
class Solution:
	def build(self, preorder:List[int],pl:int, llen:int, inorder:List[int], il:int) -> Optional[TreeNode]:
		if llen == 1:
			n = TreeNode(preorder[pl])
			return n
		
		n = TreeNode(preorder[pl])
		i = il
		for i in range(il, il+llen):
			if inorder[i] == preorder[pl]:
				break
		if i == il:
			n.left = None
		else:
			n.left = self.build(preorder,pl+1, i-il, inorder, il)
		
		if i == il+llen-1:
			n.right = None
		else:
			n.right = self.build(preorder, pl+1+i-il, il+llen-i-1, inorder, i+1)
		return n

	def buildTree(self, preorder: List[int], inorder: List[int]) -> Optional[TreeNode]:
		return self.build(preorder, 0, len(preorder), inorder, 0)

```
