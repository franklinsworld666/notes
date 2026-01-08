
## 题目

https://leetcode.cn/problems/binary-search-tree-iterator/description/?envType=problem-list-v2&envId=tree

## tag 
二叉树
中序遍历


## soulution1

+ 采用染色法
+ 首先，把根节点染色为白色，入栈
+ 然后，探测栈顶节点的颜色，如果是灰色，终止操作
+ 如果是白色，出栈，放入该节点的右孩子（白色），该节点（灰色），该节点的左孩子（白色）
+ hasnext: next为None，栈非空；或者 next不是None，栈中元素超过1；
+ next：next 初值为None；如果next为None，探测到第一个灰色节点，next指向该节点；如果next非None，栈顶节点出栈，继续探测到第一个灰色节点，next指向该节点；


time:O(n)， 操作的均摊复杂度 O(1)

space:O(logn)




## code 
```python3
class BSTIterator:

    def __init__(self, root: Optional[TreeNode]):
        self.nextPtr = None 
        self.que = collections.deque()
        if root:
            self.que.append((root, 0))
        

    def next(self) -> int:
        if self.nextPtr:
            self.que.pop()
        while True:
            node,color = self.que[-1]
            if color == 1:
                self.nextPtr = node
                return node.val
            
            node,color = self.que.pop()
            if node.right:
                self.que.append((node.right, 0))
            self.que.append((node, 1))
            if node.left:
                self.que.append((node.left, 0))
            
            
    def hasNext(self) -> bool:
        if not self.nextPtr:
            return len(self.que) != 0
        return len(self.que) > 1

```