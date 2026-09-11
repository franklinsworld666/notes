
 //基于快速划分的k选取算法
 // 最坏情况下：O(n*n)
 // 平均 : O(nlgn)
template <typename T> 
void quicksearch( Vector<T>& A, Rank k ) 
{
   for ( Rank lo = 0, hi = A.size(); lo < hi; ) 
   {
      Rank i = lo;
      Rank j = hi; 
      T pivot = A[lo]; //大胆猜测

      while ( i < j ) {
         do j--; while ( ( i < j ) && ( pivot <= A[j] ) );
         if ( i < j ) A[i] = A[j];
         do i++; while ( ( i < j ) && ( A[i] <= pivot ) );
         if ( i < j ) A[j] = A[i];
      } // assert: quit with i == j or j+1
      A[j] = pivot; // A[0,j) <= A[j] <= A(j, n)

      if ( k <= j ) hi = j; 
      if ( i <= k ) lo = i; //prefix trimmed
      
   }
}
