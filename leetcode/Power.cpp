
// a^n算法：n >= 0
// 时间：O(logn)
int64_t power (int64_t a, int n ) 
{ 
   int64_t ret = 1;
   int64_t p = a; 
   while ( 0 < n ) 
   { 
      if ( n & 1 )
         ret *= p;
      n >>= 1; //O(1)
      p *= p; //O(1)
   }
   return ret; 
}
