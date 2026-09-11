
/**
	index  0   1  2  3  4  5  6  7
  string   a   b  a  b  a  b  c  a
	pmt    0   0  1  2  3  4  0  1
	next   -1  0  0  1  2  3  4  0
next_more  -1  0  -1 0	-1 0  4  -1

	pmt  前缀和后缀中，匹配的字符串，其中最大的长度
	next 数组刚好是 pmt 右移一位 ;因为当 j 位不匹配时，我需要参考 pmt[j-1]来移动

	next 怎么来？
	algo_202402151430.png

	分摊复杂度：O(N)
**/

void getNext(char *p, int *next)
{
	next[0] = -1;

	int m = strlen(p);
	int j = 0;  // 代表后缀子串
	int k = -1;  // 代表前缀子串

	while (j < m-1) 
	{
		if ( k < 0 || p[j] == p[k])
		{
			next[j+1] = k+1; 
			++k;++j;
		}
		else
		{
			k = next[k]; // 要在 k 长度的前缀子串中，移动到最大匹配的前缀与后缀
		}
	}
 }



// 进一步优化，不仅要求移动到最大前后缀匹配的位置
// 而且，下一个位置要希望不要重复同样的错误
void getNext_more(char *p, int *next)
{
	next[0] = -1;

	int m = strlen(p);
	int j = 0;  // 代表后缀子串
	int k = -1;  // 代表前缀子串

	while (j < m-1) 
	{
		if ( k < 0 || p[j] == p[k])
		{
			// 移动到的位置，仍然可能和当前位置一样，造成无效比较
			next[j+1] = (p[j+1] != p[k+1] ? k+1: next[k+1]);  
			++k;
			++j;
		}
		else
		{
			k = next[k]; 
		}
	}
 }

// 复杂度, 令 f = 2i -j, 
// 于是 f 是单调递增，且 f < 2 * strlen(t)
// 所以，O(2n)
int KMP(char *t,char *p)
{
    int i=0;
    int j=0;
    while(i<strlen(t) && j<strlen(p))
	{ 
        if(j < 0 || t[i]==p[j] )
		{
            i++;j++;
        }else
            j=next[j];
   }
    if(j==strlen(p))  // match
        return i-j;
    else
        return -1;
}