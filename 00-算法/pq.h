
 //优先级队列PQ模板类
template <typename T> struct PQ {
   virtual void insert( T ) = 0; 
   virtual T getMax() = 0;
   virtual T delMax() = 0; //删除优先级最高的词条
};

//PQ[i]的父节点（floor((i-1)/2)，i无论正负）
#define  Parent(i)         ( ( ( i ) - 1 ) >> 1 ) 
#define  LChild(i)         ( 1 + ( ( i ) << 1 ) ) //PQ[i]的左孩子
#define  RChild(i)         ( ( 1 + ( i ) ) << 1 ) //PQ[i]的右孩子

//判断PQ[i]是否合法
#define  InHeap(n, i)      ( ( ( -1 ) != ( i ) ) && ( ( i ) < ( n ) ) ) 
#define  LChildValid(n, i) InHeap( n, LChild( i ) ) //判断PQ[i]是否有一个（左）孩子
#define  RChildValid(n, i) InHeap( n, RChild( i ) ) //判断PQ[i]是否有两个孩子

#define  Bigger(PQ, i, j)  ( lt( PQ[i], PQ[j] ) ? j : i ) //取大者（等时前者优先）

 /*父子（至多）三者中的大者*/
 //相等时父节点优先，如此可避免不必要的交换
#define  ProperParent(PQ, n, i) \
            ( RChildValid(n, i) ? Bigger( PQ, Bigger( PQ, i, LChild(i) ), RChild(i) ) : \
            ( LChildValid(n, i) ? Bigger( PQ, i, LChild(i) ) : i \
            ) \
            ) 


 //完全二叉堆
template <typename T> 
struct PQ_ComplHeap : public PQ<T>, public Vector<T> 
{
   PQ_ComplHeap() {} //默认构造
   PQ_ComplHeap( T* A, Rank n ) { copyFrom( A, 0, n ); heapify( _elem, n ); } //批量构造

   void insert( T ); //按照比较器确定的优先级次序，插入词条
   T getMax(); //读取优先级最高的词条
   T delMax(); //删除优先级最高的词条
}; 



//对向量前n个词条中的第i个实施下滤，i < n
//返回下滤抵达的位置
template <typename T> 
Rank percolateDown( T* A, Rank n, Rank i ) 
{
   while (true) {
		Rank j = ProperParent(A, n, i); // i 及孩子中的最大值
		if (i == j) break;
		swap( A[i], A[j] );
		i = j;
   }
   return j;
}


//对词条A[i]做上滤，0 <= i < _size
//返回上滤最终抵达的位置
template <typename T> 
Rank percolateUp( T* A, Rank i ) 
{ 
   while ( 0 < i ) { //在抵达堆顶之前，反复地
      Rank j = Parent( i ); //考查[i]之父亲[j]
      if ( lt( A[i], A[j] ) ) break; //一旦父子顺序，上滤旋即完成；否则
      swap( A[i], A[j] ); i = j; //父子换位，并继续考查上一层
   }
   return i; //返回上滤最终抵达的位置
}


//Floyd建堆算法，O(n)时间
// 在一棵完全二叉树中，越是下层节点越多，所以下滤会比上滤操作少得多
template <typename T> 
void heapify( T* A, Rank n )
{
   for ( Rank i = n / 2 - 1; - 1 != i; i-- ) //自底而上，依次
      percolateDown( A, n, i ); //经下滤合并子堆
}


template <typename T> 
T PQ_ComplHeap<T>::getMax() 
{
	return _elem[0]; 
}

template <typename T> 
void PQ_ComplHeap<T>::insert( T e ) 
{ //将词条插入完全二叉堆中
   Vector<T>::insert( e ); //将新词条接至向量末尾
   percolateUp( _elem, _size - 1 ); //再对该词条实施上滤调整
}


template <typename T> 
T PQ_ComplHeap<T>::delMax() 
{ //取出最大词条
   swap( _elem[0], _elem[--_size] ); //堆顶、堆尾互换（_size的递减，不致引发shrink()）
   percolateDown( _elem, _size, 0 ); //新堆顶下滤
   return _elem[_size]; //返回原堆顶
}


