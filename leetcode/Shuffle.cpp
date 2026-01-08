

// 将A[0, n)随机打乱
// 例如：随机选择一个数放在 (n-1)位置，就可以 size-1
void shuffle( int A[], int n ) 
{ 
   for ( ; 1 < n; --n ) //自后向前
      swap( A[rand() % n], A[n - 1] );
}
