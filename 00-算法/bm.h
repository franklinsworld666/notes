/*
 BM_BC 算法：是一个善于利用经验的算法， bad character
假设 S 中的字符 'c' 与 P 中的 't' 不匹配，那么选择把 P 向右移动，使得 P 中最右边的 'c' 与 S 的'c' 匹配；


 最好情况下：O(N/m)
 字母表越是大，单个字符匹配成功概率越低，BM算法性能越高；

 最坏情况下：O（N*m）
 退化成了蛮力算法


BC+GS算法：利用好的经验，good suffix
假设 S 中 "abc" 与 P 的后缀 "abc" 匹配了，那么P向右移动时，移动到倒数第2个存在 "abc" 的地方


最好的情况：O(N/m)
最坏的情况：O(n+m)
	

*/


int match ( char* P, char* T ) { //Boyer-Morre算法(简化版，只考虑Bad Character Shift)
   int* bc = buildBC ( P ); //预处理
   size_t n = strlen ( T ), i = 0; //文本串长度、与模式串首字符的对齐位置
   size_t m = strlen ( P ); //模式串长度
   while ( n >= i + m ) { //在到达最右端前，不断右移模式串（可能不止一个字符）
      int j = m - 1; //从模式串最末尾的字符开始
      while ( P[j] == T[i+j] ) //自右向左比对
         if ( 0 > --j ) break;
      if ( j < 0 ) //若极大匹配后缀 == 整个模式串，则说明已经完全匹配，故
         break; //返回匹配位置
      else //否则，根据BC表
         i += max ( 1, j - bc[T[i+j]] ); //相应地移动模式串，使得T[i+j]与P[bc[T[i+j]]]对齐
   }
   delete [] bc; //销毁BC表
   return i;
}

/******************************************************************************************
 *    0                       bc['X']                                m-1
 *    |                       |                                      |
 *    ........................X***************************************
 *                            .|<------------- 'X' free ------------>|
 ******************************************************************************************/
int* buildBC( char* P ) { //构造Bad Charactor Shift表：O(m + 256)
   int* bc = new int[256]; // BC表，与字符表等长
   for ( size_t j = 0; j < 256; j++ ) bc[j] = -1; //初始化：首先假设所有字符均未在P中出现
   for ( size_t m = strlen( P ), j = 0; j < m; j++ ) //自左向右扫描模式串P
      bc[P[j]] = j; //将字符P[j]的BC项更新为j（单调递增）——画家算法
   return bc;
}

 //Boyer-Morre算法（完全版，兼顾Bad Character与Good Suffix）
int match ( char* P, char* T ) {
   int* bc = buildBC ( P ); int* gs = buildGS ( P ); //构造BC表和GS表
   size_t i = 0; //模式串相对于文本串的起始位置（初始时与文本串左对齐）
   while ( strlen ( T ) >= i + strlen ( P ) ) { //不断右移（距离可能不止一个字符）模式串
      int j = strlen ( P ) - 1; //从模式串最末尾的字符开始
      while ( P[j] == T[i + j] ) //自右向左比对
         if ( 0 > --j ) break;
      if ( 0 > j ) //若极大匹配后缀 == 整个模式串（说明已经完全匹配）
         break; //返回匹配位置
      else //否则，适当地移动模式串
         i += max ( gs[j], j - bc[ T[i + j] ] ); //位移量根据BC表和GS表选择大者
   }
   delete [] gs; delete [] bc; //销毁GS表和BC表
   return i;
}


 // 构造最大匹配后缀长度表：O(m)
 // 单调性：考察 j + lo, 情况一: j减少1; 情况二：j减少1，并且lo不增加；最后 j = 0, lo = 0
 // 不变性：ss[j+1, m) 都是已经计算出来的值
 // 最多进行 O(m)

 /*
   b  a  n  a  n  a
   0  1  0  3  0  6     
 */
int* buildSS ( char* P ) {
   int m = strlen ( P ); 
   int* ss = new int[m];

   //对最后一个字符而言，与之匹配的最长后缀就是整个P串
   ss[m - 1]  =  m; 
   
   // 以下，从倒数第二个字符起自右向左扫描P，依次计算出ss[]其余各项
   for ( int lo = m - 1, hi = m - 1, j = lo - 1; j >= 0; j -- ) {
      //情况一:直接利用此前已计算出的ss[], (lo, hi]已经匹配成功了，利用这个信息
      if ( ( lo < j ) && ( ss[m - hi + j - 1] < j - lo ) ) 
         ss[j] =  ss[m - hi + j - 1]; 
      else { //情况二
         hi = j; lo = min ( lo, hi );
         while ( ( 0 <= lo ) && ( P[lo] == P[m - hi + lo - 1] ) )
            lo--; //逐个对比处于(lo, hi]前端的字符
         ss[j] = hi - lo;
      }
   }
   return ss;
}


 //构造好后缀位移量表：O(m)
 // 希望移动最少必要的距离
int* buildGS ( char* P ) {
   int* ss = buildSS ( P ); //Suffix Size table
   size_t m = strlen ( P ); 
   int* gs = new int[m]; //Good Suffix shift table
   
   // m 是 gs[j] 的一种选择
   for ( size_t j = 0; j < m; j ++ ) 
      gs[j] = m; 
   
    //逆向逐一扫描各字符
    // 针对前缀 == 某个后缀 的情况 P[0, j] = P[m - j - 1, m)，则
    // 对 0 <= i < m-j-1, gs[i] = m-j-1
    // 从后往前计算，只是为了不重复计算
   for ( size_t i = 0, j = m - 1; j < UINT_MAX; j-- ) {
      if ( j + 1 == ss[j] ) 
         while ( i < m - j - 1 ) 
            gs[i++] = m - j - 1;
   }

   //画家算法：正向扫描P[]各字符，gs[j]不断递减，直至最小
   //m - j - 1必是其gs[m - ss[j] - 1]值的一种选择
   for ( size_t j = 0; j < m - 1; j ++ ) {
      gs[m - ss[j] - 1] = m - j - 1; 
   }
   delete [] ss; return gs;
}