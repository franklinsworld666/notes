

// 插入排序
// 特点：输入敏感型，与逆序对数量相关
template <typename T> 
void Vector<T>::insertionSort( Rank lo, Rank hi ) 
{
   for ( Rank j = lo + 1; j < hi; j++ ) 
   {
      Rank i = j; 
      T x = _elem[i];   // 待排序元素
      // 不断和前一个元素比较，找到合适的位置
      while ( ( lo + 1 <= i ) && ( _elem[i] < _elem[i - 1] ) ) {
         _elem[i] = _elem[i - 1];
         i--;
      }
      _elem[i] = x;
   }
}


// 归并排序
// 归并排序，要点在于：合并
// 时间: O(nlgn)
// 空间: O(n)
// 稳定性：稳定
template <typename T> 
void Vector<T>::mergeSort( Rank lo, Rank hi ) 
{ // 0 <= lo < hi <= size

   if ( hi - lo < 2 ) return; 
   Rank mi = ( lo + hi ) / 2; //以中点为界
   mergeSort( lo, mi ); 
   mergeSort( mi, hi ); 
   merge( lo, mi, hi );
}


// shell sort
// shell sort 受 序列 影响大
// PS Sequence: { 1, 3, 7, 15, 31, ... }
template <typename T> 
void Vector<T>::shellSort( Rank lo, Rank hi ) 
{
   for ( Rank d = 0x7FFFFFFF; 0 < d; d >>= 1 ) 
   {
      for ( Rank j = lo + d; j < hi; j++ ) 
      {
         Rank i = j; 
         T x = _elem[i];   // 待排序元素
         // 不断和前一个元素比较，找到合适的位置
         while ( ( lo + d <= i ) && ( _elem[i] < _elem[i - d] ) ) {
            _elem[i] = _elem[i - d];
            i -= d;
         }
         _elem[i] = x;
      }
   }
}

// heap sort


// bucket sort


// radix sort


// counting sort


// 快排
// 快排，要点在于：分成2部分， L(x) <= pivot <= R(x)
// 性能：最好 O(nlgn)
// 性能：平均 O(n^2)
// 降低最坏情况：调整轴点算法，比如：取一个随机数
#define Put( K, s, t ) { if ( 1 < (t) - (s) ) { K.push(s); K.push(t); } }
#define Get( K, s, t ) { t = K.pop(); s = K.pop(); }

template <typename T> 
void Vector<T>::quickSort( Rank lo, Rank hi ) 
{ //0 <= lo < hi <= size
   Stack<Rank> Task; 
   Put( Task, lo, hi ); 
   while ( !Task.empty() ) {
      Get( Task, lo, hi ); 
      Rank mi = partition( lo, hi ); //在[lo, hi)内构造轴点
      if ( mi - lo < hi - mi ) { Put( Task, mi+1, hi ); Put( Task, lo, mi ); }
      else                     { Put( Task, lo, mi ); Put( Task, mi+1, hi ); }
   } //大任务优先入栈（小任务优先出栈执行），可保证递归深度（空间成本）不过O(logn)
}

//通过调整元素位置，构造出区间[lo, hi)内的一个轴点
// DUP版：可优化处理多个关键码雷同的退化情况
template <typename T> 
Rank Vector<T>::partition( Rank lo, Rank hi ) 
{
   swap( _elem[lo], _elem[lo + rand() % ( hi - lo )] );
   T pivot = _elem[lo]; 

   while ( lo < hi ) {
      do {hi--; }while ( ( lo < hi ) && ( pivot < _elem[hi] ) ); //向左拓展后缀G
      if ( lo < hi ) _elem[lo] = _elem[hi]; //阻挡者归入前缀L

      do {lo++; } while ( ( lo < hi ) && ( _elem[lo] < pivot ) ); //向右拓展前缀L
      if ( lo < hi ) _elem[hi] = _elem[lo]; //阻挡者归入后缀G
   } // assert: quit with lo == hi or hi + 1
   
   _elem[hi] = pivot; 
   
   return hi; //返回轴点的秩
}
