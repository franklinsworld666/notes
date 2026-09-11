
## 题目
https://leetcode.cn/problems/basic-calculator/

## tag 
栈
中缀表达式

## 思路
一个栈存放数字，一个栈存放运算符；
遍历字符串，遇到数字时放入运算数栈，遇到运算符就比较栈顶和当前运算符，根据结果来处理

时间复杂度: O(N)
空间复杂度: O(N)

不变性：
单调性：


## 代码
```python3
#!/usr/bin/python3
# -*- coding:utf-8 -*-

from typing import List
from typing import Tuple

class Solution:
	def is_binary(self, s:str, i:int) -> bool:
		while i >= 0:
			if s[i].isdigit():
				return True
			elif s[i] == '(':
				return False
			elif s[i] == ')':
				return True
			else:
				i -= 1
		return False


	def read_num(self, s:str, i:int) -> Tuple[int,int]:
		l = i
		while i < len(s):
			if not s[i].isdigit():
				break
			i += 1
		num = int(s[l:i])
		return num,i
	
	"""
	()
	+ - |
	\0
	"""
	def cmp_process(self, s:str, op:List[str], nums:List[int]) -> None:
		if not op:
			op.append(s)
		if s == "(":
			op.append(s)
		elif s == ")":
			if op[-1] == "+":
				op.pop()
				op.pop()
				n = nums[-2] + nums[-1]
				nums.pop()
				nums[-1] = n
			elif op[-1] == "-":
				op.pop()
				op.pop()
				n = nums[-2] - nums[-1]
				nums.pop()
				nums[-1] = n
			elif op[-1] == "|":
				op.pop()
				op.pop()
				nums[-1] = (-1) * nums[-1]
		else:
			if s == "+":
				op.pop()
				n = nums[-2] + nums[-1]
				nums.pop()
				nums[-1] = n
				op.append(s)
			elif s == "-":
				op.pop()
				n = nums[-2] - nums[-1]
				nums.pop()
				nums[-1] = n
				op.append(s)
			elif s == "|":
				op.pop()
				n = (-1) * nums[-1]
				nums[-1] = n
				op.append(s)
			else:
				op.append(s)
				

	def calculate(self, s: str) -> int:
		num_stack = []
		op_stack = []
		i = 0
		while i < len(s):
			if s[i] == ' ':
				i += 1
			elif s[i].isdigit():
				num,i = self.read_num(s,i)
				num_stack.append(num)
			elif s[i] == '-':
				binary_op = self.is_binary(s, i)
				if not binary_op:
					self.cmp_process("|", op_stack, num_stack)
				else:
					self.cmp_process(s[i], op_stack, num_stack)
				i += 1
			else:
				self.cmp_process(s[i], op_stack, num_stack)
				i += 1
		self.cmp_process("\0", op_stack, num_stack)
		return num_stack[0]
```