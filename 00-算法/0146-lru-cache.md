
## 题目

https://leetcode.cn/problems/lru-cache/description/

## tag 
哈希表
双链表


## soulution1
get 操作要实现 O(1), 所以用 哈希表(python3 中 dict);

put 操作实现为一个双链表


时间复杂度: O(1)
空间复杂度: O(N)

## code 
```python3

class ListNode:
	def __init__(self, key:int, value:int):
		self.key = key
		self.val = value
		self.pre = self
		self.next = self

class LRUCache:
	def removeFromList(self, node:ListNode):
		pre = node.pre
		next = node.next
		pre.next = next
		next.pre = pre

	def appendList(self, node:ListNode):
		last = self.head.pre
		last.next = node
		node.pre = last
		node.next = self.head
		self.head.pre = node

	def __init__(self, capacity: int):
		self.capacity = capacity
		self.size = 0
		self.mydict = {}
		self.head = ListNode(0,0)



	def get(self, key: int) -> int:
		if key in self.mydict:
			node = self.mydict[key]
			self.removeFromList(node)
			self.appendList(node)
			return node.val
		else:
			return -1


	def put(self, key: int, value: int) -> None:
		if key in self.mydict:
			node = self.mydict[key]
			node.val = value
			self.removeFromList(node)
			self.appendList(node)
			return
		else:
			if self.size >= self.capacity:
				node = self.head.next
				self.removeFromList(node)
				self.mydict.pop(node.key)
				self.size -= 1
			node = ListNode(key, value)
			self.appendList(node)
			self.mydict[key] = node
			self.size += 1
			


```