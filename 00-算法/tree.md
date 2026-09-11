
[TOC]

## 基本概念
 树，是一类特殊的图，T = (V, E), 节点数|V| = n, 边数 |E| = e;
 
 父亲，孩子，兄弟sibling，
 d(r) = degree(r), 树中节点 r 的出度
 树中的边数 e = Sum(d(r)) = n - 1 = O(n), 即树的规模等价于 O(n)

有序树：在兄弟之间，也定义了明确顺序的树；

路径 path：k+1 个顶点，通过 k 条边依次相连；
路径长度 = 边数 k；
环路cycle/loop : 在一条路径中，起点顶点 和 结束顶点，是同一个；

连通图：每2个节点之间，都存在路径；
无环图：图中没有环路；

树的特性：节点V的父亲唯一，后代不唯一；连通无环图，极小连通图，极大无环图；  ---> 每一个节点到根节点，存在唯一的路径；

节点 V 的深度：path(V) = 根节点到节点 V 的路径的长度；
树的深度：所有叶子节点的深度中的最大值；

二叉树：每一个节点的出度，不超过2；
真二叉树：每一个节点的出度，或者是2，或者是0；
任何一颗多叉树，都可以采用孩子兄弟表示法，然后通过旋转变为一颗二叉树；

完全二叉树
二叉搜索树
二叉平衡搜索树：B树，splay树，RB树


## 数据结构
```cpp
typedef unsigned int Rank;

#if defined( DSA_REDBLACK ) //在红黑树中
	#define stature( p ) ( ( p ) ? ( p )->height : 0 ) //外部节点（黑）高度为0
#else //其余BST中
	#define stature( p ) ( (int)( ( p ) ? ( p )->height : -1 ) ) //外部节点高度为-1
#endif


#define IsRoot(x) ( ! ( (x).parent ) )
#define IsLChild(x) ( ! IsRoot(x) && ( & (x) == (x).parent->lc ) )
#define IsRChild(x) ( ! IsRoot(x) && ( & (x) == (x).parent->rc ) )
#define HasParent(x) ( ! IsRoot(x) )
#define HasLChild(x) ( (x).lc )
#define HasRChild(x) ( (x).rc )
#define HasChild(x) ( HasLChild(x) || HasRChild(x) ) //至少拥有一个孩子
#define HasBothChild(x) ( HasLChild(x) && HasRChild(x) ) //同时拥有两个孩子
#define IsLeaf(x) ( ! HasChild(x) )
#define sibling( p ) ( IsLChild( * (p) ) ? (p)->parent->rc : (p)->parent->lc ) /*兄弟*/
#define uncle( x ) ( sibling( (x)->parent ) ) /*叔叔*/
#define FromParentTo( x ) /*来自父亲的引用*/ \
   ( IsRoot(x) ? _root : ( IsLChild(x) ? (x).parent->lc : (x).parent->rc ) )


typedef enum { RB_RED, RB_BLACK} RBColor; 

template <typename T> struct BinNode;
template <typename T> using BinNodePosi = BinNode<T>*; 

template <typename T> struct BinNode { 
   T data; 
   BinNodePosi<T> parent, lc, rc; //父节点及左、右孩子
   Rank height; //高度（通用）
   Rank npl; //Null Path Length（左式堆，也可直接用height代替）
   RBColor color; //颜色（红黑树）


// 构造函数
   BinNode() : parent( NULL ), lc( NULL ), rc( NULL ), height( 0 ), npl( 1 ), color( RB_RED ) {}

   BinNode( T e, BinNodePosi<T> p = NULL, BinNodePosi<T> lc = NULL,
            BinNodePosi<T> rc = NULL, int h = 0, int l = 1, RBColor c = RB_RED )
      : data( e ), parent( p ), lc( lc ), rc( rc ), height( h ), npl( l ), color( c ) {}


// 操作接口
   Rank size(); //统计当前节点后代总数，亦即以其为根的子树的规模
   BinNodePosi<T> insertAsLC( T const& ); //作为当前节点的左孩子插入新节点
   BinNodePosi<T> insertAsRC( T const& ); //作为当前节点的右孩子插入新节点
   BinNodePosi<T> succ(); //中序遍历下，当前节点的直接后继
   template <typename VST> void travLevel( VST& ); //子树层次遍历
   template <typename VST> void travPre( VST& ); //子树先序遍历
   template <typename VST> void travIn( VST& ); //子树中序遍历
   template <typename VST> void travPost( VST& ); //子树后序遍历

// 比较器、判等器（各列其一，其余自行补充）
   bool operator< ( BinNode const& bn ) { return data < bn.data; } //小于
   bool operator== ( BinNode const& bn ) { return data == bn.data; } //等于
};

template <typename T>
Rank BinNode<T>::size()
{
	int s = 1;
	if (lc) s += lc->size();
	if (rc) s += rc->size();
	return s;
}

template <typename T>
BinNodePosi<T> BinNode<T>::insertAsLC(T const &e)
{
	return lc = new BinNode<T>(e,this);
}

template <typename T>
BinNodePosi<T> BinNode<T>::insertAsRC(T const &e)
{
	return rc = new BinNode<T>(e,this);
}
```



## 前序遍历
```cpp
// 白色：没有访问过，遇到白色标记为灰色，然后将自己和子节点入栈
// 灰色：访问过，遇到灰色则直接输出值
void travPre_I2( BinNodePosi<T> x, VST& visit ) {
   Stack<int, BinNodePosi<T>> S; 

   S.push(<white, x>);

   while (!S.empty()) {
      x = S.pop();
      if (x is white){
         if (x->rc) S.push(<white,x->rc>);
         if (x->lc) S.push(<white,x->lc>);
         S.push(<gray, x>);
      }
      else {
         visit(x);
      }
   }

}
```

## 中序遍历
```cpp
// 白色：没有访问过，遇到白色标记为灰色，然后将自己和子节点入栈
// 灰色：访问过，遇到灰色则直接输出值
void travIn_I2( BinNodePosi<T> x, VST& visit ) {
   Stack<int, BinNodePosi<T>> S; 

   S.push(<white, x>);

   while (!S.empty()) {
      x = S.pop();
      if (x is white){
         if (x->rc) S.push(<white,x->rc>);
         S.push(<gray, x>);         
         if (x->lc) S.push(<white,x->lc>);
      }
      else {
         visit(x);
      }
   }

}


void travIn_I1( BinNodePosi<T> x, VST& visit ) {
   Stack<BinNodePosi<T>> S; 
   while ( true ) {
	  
	  while (x) {
		S.push(x);
		x = x->lc;
	  }

      if ( S.empty() ) break; //直至所有节点处理完毕
      x = S.pop(); visit( x->data ); //弹出栈顶节点并访问之
      x = x->rc; //转向右子树
   }
}
```

## 后序遍历
```cpp
// 白色：没有访问过，遇到白色标记为灰色，然后将自己和子节点入栈
// 灰色：访问过，遇到灰色则直接输出值
void travPost_I2( BinNodePosi<T> x, VST& visit ) {
   Stack<int, BinNodePosi<T>> S; 

   S.push(<white, x>);

   while (!S.empty()) {
      x = S.pop();
      if (x is white){
         S.push(<gray, x>);         
         if (x->rc) S.push(<white,x->rc>);
         if (x->lc) S.push(<white,x->lc>);
      }
      else {
         visit(x);
      }
   }

}


// 在以S的栈顶节点为根的子树中，找到最高左侧可见叶节点
template <typename T> 
static void gotoLeftmostLeaf( Stack<BinNodePosi<T>>& S ) 
{
	BinNodePosi<T> x;
   while ( x = S.top() ) {
      if ( HasLChild( *x ) ) { //尽可能向左
         if ( HasRChild( *x ) ) S.push( x->rc ); //若有右孩子，优先入栈
         S.push( x->lc ); //然后才转至左孩子
      } else //实不得已
         S.push( x->rc ); //才向右
   }
   S.pop(); //返回之前，弹出栈顶的空节点
}

template <typename T, typename VST>
void travPost_I( BinNodePosi<T> x, VST& visit )
{
   Stack<BinNodePosi<T>> S; 
   if ( x ) S.push( x ); 

   while ( !S.empty() ) {
      if ( S.top() != x->parent ) ////若栈顶非x之父（而为右兄）
         gotoLeftmostLeaf( S ); //则在其右兄子树中找到HLVFL（相当于递归深入）
      x = S.pop(); visit( x->data ); 
   }
}

```

## 层次遍历/BFS
```python3
class Solution:
    def bfs(k):
        queue = collections.deque([root])
        # 记录层数
        steps = 0
        # 需要返回的节点
        ans = []
        # 队列不空，生命不止！
        while queue:
            size = len(queue)
            # 遍历当前层的所有节点
            for _ in range(size):
                node = queue.popleft()
                if (step == k) ans.append(node)
                if node.right:
                    queue.append(node.right)
                if node.left:
                    queue.append(node.left)
            # 遍历完当前层所有的节点后 steps + 1
            steps += 1
        return ans
```

## DFS
```javascript
# 其中 path 是树的路径， 如果需要就带上，不需要就不带
def dfs(root, path):
    # 空节点
    if not root: return
    # 叶子节点
    if not root.left and not root.right: return
    
    path.append(root)
    # 逻辑可以写这里，此时是前序遍历
    dfs(root.left)
    dfs(root.right)
    # 需要弹出，不然会错误计算。
    # 比如对于如下树：
    """
              5
             / \
            4   8
           /   / \
          11  13  4
         /  \    / \
        7    2  5   1
    """
    # 如果不 pop，那么 5 -> 4 -> 11 -> 2 这条路径会变成 5 -> 4 -> 11 -> 7 -> 2，其 7 被错误地添加到了 path

    path.pop()
    # 逻辑也可以写这里，此时是后序遍历

    return 你想返回的数据
```


## 寻找当前节点的中序后继
```cpp
template <typename T>
BinNodePosi<T> BinNode<T>::succ()
{
	BinNodePosi<T> s = this;
	if (s->rc) {
		s = rc;
		while (s->lc) s = s->lc;
	}
	else {
		if (IsLChild(*s)) return s->parent;

		while(IsRChild(*s)) s = s->parent;
		return s->parent;
	}
	return s;
}

```
