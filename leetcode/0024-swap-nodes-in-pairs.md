
## 题目

24 https://leetcode.cn/problems/swap-nodes-in-pairs/description/


## tag
链表
减治
递归
迭代


## solution1
减枝，用递归来实现。先处理前2个节点，然后递归处理后续

## solution2
用递归实现，出口：链表为空，或只有一个节点

## code 
```python3

class ListNode:
    def __init__(self, val=0, next=None):
        self.val = val
        self.next = next

class Solution:
    def swapPairs(self, head: Optional[ListNode]) -> Optional[ListNode]:
        if not head or not head.next:
            return head
        second = head.next
        third = second.next
        head.next = self.swapPairs(third)
        second.next = head
        head = second
        return head


```


```python3
class Solution:
    def swapPairs(self, head: Optional[ListNode]) -> Optional[ListNode]:
        if not head or not head.next:
            return head
        pre = None
        first = head
        head = head.next
        while first and first.next:
			# 直接就位即可
            second = first.next
            third = second. next

            if pre:
                pre.next = second		
			second.next = first	
            first.next = third
			
            pre = first
            first = third
        return head
```