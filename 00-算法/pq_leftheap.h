/*
	为什么要设计左式堆？
	针对2个堆有序的优先级队列，进行堆合并。
	如果采用把堆B中的元素逐一插入堆A的办法，复杂度O(log(n+m)), 显然并没有利用到堆的有序性

	左式堆：节点分布偏向于左侧，合并操作只涉及右侧。
	为了度量左式堆的单侧倾斜性，为了确保右侧高度O(logn),引入 空节点路径长度NPL。
	npl(NULL) = 0;
	npl( v ) = 1 + min(npl(v->lc), npl(v->rc));
	==》  npl(v) : v到外部节点的最近距离， 以v为根的最大满子树的高度
	
	左倾性：npl(v->lc) >= npl(v->rc)
	==>  npl(v) = 1 + npl(v->rc)


*/


template <typename T>
class PQ_LeftHeap : public PQ<T>, public BinTree<T> { //基于二叉树，以左式堆形式实现的PQ
 public:
   PQ_LeftHeap() {} //默认构造
   PQ_LeftHeap( T* E, int n ) //批量构造：可改进为Floyd建堆算法
    { for ( int i = 0; i < n; i++ ) insert( E[i] ); }

   PQ_LeftHeap( PQ_LeftHeap& A, PQ_LeftHeap& B ) { //合并构造
      _root = merge( A._root, B._root ); _size = A._size + B._size;
      A._root = B._root = NULL; A._size = B._size = 0;
   }

   void insert( T ); //按照比较器确定的优先级次序插入元素
   T getMax(); //取出优先级最高的元素
   T delMax(); //删除优先级最高的元素
}; 


template <typename T> 
T PQ_LeftHeap<T>::getMax() { return _root->data; }


//将e封装为左式堆，与当前左式堆合并
template <typename T> 
void PQ_LeftHeap<T>::insert( T e ) 
{
   _root = merge( _root, new BinNode<T>( e, NULL ) ); 
   _size++; //更新规模
}


template <typename T> 
T PQ_LeftHeap<T>::delMax() 
{
   BinNodePosi<T> lHeap = _root->lc; if ( lHeap ) lHeap->parent = NULL; //左子堆
   BinNodePosi<T> rHeap = _root->rc; if ( rHeap ) rHeap->parent = NULL; //右子堆
   T e = _root->data; delete _root; _size--; //删除根节点
   _root = merge( lHeap, rHeap ); //合并原左、右子堆
   return e; //返回原根节点的数据项
}

//合并以a和b为根节点的两个左式堆（递归版）
template <typename T> 
BinNodePosi<T> merge( BinNodePosi<T> a, BinNodePosi<T> b ) 
{
   if ( !a ) return b; //退化情况
   if ( !b ) return a; //退化情况

   // 确保根 a 是最大值
   if ( lt( a->data, b->data ) ) swap( a, b ); //确保a>=b

   ( a->rc = merge( a->rc, b ) )->parent = a; //将a的右子堆，与b合并
   
   // 保证左倾性
   if ( !a->lc || ( a->lc->npl < a->rc->npl ) ) 
      swap( a->lc, a->rc ); //交换a的左、右子堆，以确保右子堆的npl不大
   a->npl = a->rc ? a->rc->npl + 1 : 1;
   return a; //返回合并后的堆顶
}