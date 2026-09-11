
## 题目

https://leetcode.cn/problems/recover-binary-search-tree/description/?envType=problem-list-v2&envId=tree

## tag 
二叉树
中序遍历

## soulution1

+ 对树进行中序遍历
+ 中序遍历之后的结果应该是有序的，如果发生了元素交换，那么肯定就无序了，找出这2个无序的元素
+ 从左往右，找出第一个大于后序的元素；从右往左，找出第一个小于前序的元素

time:O(n)

space:O(n)

## soulution2

+ 对树进行中序遍历
+ 中序遍历之后的结果应该是有序的，如果发生了元素交换，那么肯定就无序了，找出这2个无序的元素
+ 从左往右，找出第一个大于后序的元素；从右往左，找出第一个小于前序的元素
+ 这里和上一个方法的区别在于：只要记录下前一个节点用于判断是否乱序即可，减少了空间占用

time:O(n)

space:O(logn)

## code 
```python3
class Solution:
    def recoverTree(self, root: Optional[TreeNode]) -> None:
        ans = []
        stack = []
        if not root:
            return 
        stack.append((root, 0))
        while stack:
            item = stack.pop()
            if item[1] == 0:
                if item[0].right:
                    stack.append((item[0].right, 0))
                stack.append((item[0], 1))
                if item[0].left:
                    stack.append((item[0].left, 0))
            elif item[1] == 1:
                ans.append(item[0])
        
        if len(ans) == 1:
            return
        
        t1 = 0
        t2 = 0
        for i in range(0, len(ans)):
            if ans[i] > ans[i+1]:
                t1 = i  
        for i in range(len(ans)-1, -1, -1):
            if ans[i] < ans[i-1]:
                t2 = i  
        tmp = ans[t1].val
        ans[t1].val = ans[t2].val
        ans[t2].val = tmp
        return
```


```python3
class Solution:
    def recoverTree(self, root: Optional[TreeNode]) -> None:
        stack = []
        if not root:
            return 
        pred = None
        x = None
        y = None
        stack.append((root, 0))
        while stack:
            item = stack.pop()
            if item[1] == 0:
                if item[0].right:
                    stack.append((item[0].right, 0))
                stack.append((item[0], 1))
                if item[0].left:
                    stack.append((item[0].left, 0))
            elif item[1] == 1:
                if pred and item[0].val < pred.val:
                    if not x:
                        x = pred
                        y = item[0]
                    else:
                        y = item[0]
                        break
                pred = item[0]
        if x and y:
            x.val,y.val = y.val,x.val
        return
        
        
```