#include <stdlib.h>

using Rank = unsigned int;

class Bitmap {
private:
   unsigned char* M;
   Rank N;  // 规模，字节
   Rank _sz; 

protected:
   void init( Rank n )
      { M = new unsigned char[N = ( n + 7 ) / 8]; memset( M, 0, N ); _sz = 0; }

   void expand( Rank k ) { 
      if ( k < 8 * N ) return; 
	  Rank oldsz = _sz;
      Rank oldN = N; 
	  unsigned char* oldM = M;
     
      init( 2 * k ); 
      memcpy_s( M, N, oldM, oldN );
	  _sz = oldsz;
      delete[] oldM; 
   }

public:
   Bitmap( Rank n = 8 ) { init( n ); } 

   Bitmap( char* file, Rank n = 8 ) { 
      init( n );
      FILE* fp = fopen( file, "r" ); fread( M, sizeof( char ), N, fp ); fclose( fp );
      for ( Rank k = 0, _sz = 0; k < n; k++ ) _sz += test(k);
   }

   ~Bitmap() { delete[] M; M = NULL; _sz = 0; } 

   Rank size() { return _sz; }

   // 第k位设置为1
   void set   ( Rank k ) { 
      expand( k ); 
      _sz++; 
      M[k >> 3] |=   ( 0x80 >> ( k & 0x07 ) ); 
   }

   void clear ( Rank k ) { expand( k ); _sz--; M[k >> 3] &= ~ ( 0x80 >> ( k & 0x07 ) ); }
   // 测试第 k 为是否为1
   bool test  ( Rank k ) { expand( k ); return M[k >> 3] &    ( 0x80 >> ( k & 0x07 ) ); }

   // 把 bitmap 结构写入文件
   void dump( char* file )
   {
      FILE* fp = fopen( file, "w" ); 
      fwrite( M, sizeof ( char ), N, fp ); 
      fclose( fp ); 
   }

   // 把结果输出为一个字符串，N 位bit 用 N 个字节来显示
   char* bits2string( Rank n ) 
   { 
      expand( n - 1 ); 
      char* s = new char[n + 1]; s[n] = '\0'; 
      for ( Rank i = 0; i < n; i++ ) s[i] = test( i ) ? '1' : '0';
      return s; 
   }

   // 把 bitmap 的 n个 bit 打印输出
   void print( Rank n ) 
   {  expand( n ); for ( Rank i = 0; i < n; i++ ) printf( test ( i ) ? "1" : "0" );  }
};