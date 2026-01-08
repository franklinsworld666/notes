
//统计整数二进制展开中数位1的总数：O(logn)

// 暴力算法
// 时间： O(logn)
int countOnes ( unsigned int n ) 
{ 
   int ones = 0; 
   while ( 0 < n ) {
      ones += ( 1 & n ); //检查最低位，若为1则计数
      n >>= 1;
   }
   return ones; 
}

// 时间: O( 二进制中 1 的个数)
// 每一次循环，清除当前最靠右的1
int countOnes1 ( unsigned int n ) 
{
   int ones = 0;
   while ( 0 < n ) { 
      ones++; //计数（至少有一位为1）
      n &= n - 1; //清除当前最靠右的1
   }
   return ones;
}