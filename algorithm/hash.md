

## hash 函数
把一个输入的整数、浮点数、字符串等等，转化为另一个输出的功能；

把很大的空间R 映射到 一个小的空间A，R >> A;

它需要满足的特点：
 1. 同一个 key 总是映射到同一个地址；
 2. 快速，期望 O(1) ;
 3. 均匀，key 映射到散列表各个地址的概率接近，避免聚集现象；  --- ps: 一方面是降低冲突发生的概率；另一方面，否则某些key，插入删除速度会更快，而另一些则会更慢；
 4. 满射，尽可能覆盖散列表空间；


## 设计一个 hashtable 
1. hashcode 计算：把各种数据类型，如整数、字符串、浮点数转换为 key；
2. 把 key 计算出一个 hash 值；
3. 出现冲突时怎么解决；
4. hashtable 的装填因子达到某个值，如何调整空间以降低后续冲突；

以上四部分，可以独立替换为不同方法。


### hashCode
因为很多数据类型不是整数，所以先做一次转换为整数，然后再计算hash 值；

```cpp
static int32_t hashCode( char c ) { return (int32_t)c; } 

static int32_t hashCode( int k ) { return (int32_t)k; } 

static int32_t hashCode( long long i ) {
    return (int32_t)( ( i >> 32 ) + (int)i ); 
}

// times31 算法, java源码采用这个
// 针对英文
static int32_t hashCode( char s[] ) {
   unsigned int hash = 0;
	while(*str){
		hash = ((hash << 5) - hash) + (*str++);
	}
	return (hash & 0x7FFFFFFF);
}
```


### hash 函数
为什么需要把 key 计算出一个 hash 值，因为直接采用 key 地址空间太大了。

#### 除余法
hash(key) = key % M;

M 取一个素数。

有利于均匀性：
当我们使用素数作为基数时，由于素数没有明显的倍数规律，key值的不同取值在进行取模运算后，更有可能产生不同的余数。这就像是在一个没有明显规律的网格上随机投掷点，每个点落在不同位置的概率更高。

相反，当我们使用2的幂作为基数时，由于它有固定的因子，很多key值（特别是2的倍数）会产生相同的余数。这就像是在一个有明显行和列的网格上投掷点，很多点会落在相同的行或列上，减少了分布的均匀性。

举例：M 取7或8，
7： 1,3,5...99, 模7的结果均匀分布在 [0-6]
8:  1,3,5...99, 模8的结果只有 1,3,5,7


#### MAD（mutiply-add-divide）
除余法的缺陷：
+ 不动点：不论 M 取什么值，都有 hash(0) = 0
+ 零阶均匀：相邻的 key，散列地址也是相邻的

如何达到一阶均匀：邻近的 key，散列地址不再临近？

hash(key) = (a * key + b) % M;

#### 平方取中：
hash(key) = key^2 取结果中间的3位数字；

hash(123) = 512;

#### 位异或法XOR
将 key 分割成等宽的二进制段，经异或运算得到地址
hash(110 011 011) = 110 ^ 011 ^ 011 = 110


### 冲突解决
#### 封闭地址 --- 拉链法
优点：任意多次冲突都可解决，操作实现简单、统一；

缺点：节点要动态申请（时间成本更高），指针需要额外空间；最大的缺陷，空间未必连续分布，系统缓存功能失效



#### 开放地址 --- 线性试探

首先，hash(key) = key % M;如果该地址为空，则放入该地址；如果不为空，则向后线性试探 hash(key) = (key + 1) %M;
然后，直到遇到一个空地址，成功；或者失败
最后，删除的时候，要注意不要把查找链给打断。一种方法是采用懒惰删除，当查找时遇到该标记，继续往后查找；当插入时遇到该标记，可以插入；

优点：无需额外的指针、链表等空间；查找链具有局部性，可以充分利用系统缓存，减少IO；

缺点：冲突增多，本次冲突会导致后续的冲突，操作 > O(1)

#### 开放地址 --- 平方试探
hask(key) = (key + 1^2) % M;
hask(key) = (key + 2^2) % M;
hask(key) = (key + 3^2) % M;

问题：hash地址空间有位置可以插入，如果采用平方试探，是否一定可以找到可插入的位置呢？
答案：可以，只要装填因子足够小。
若M为素数，n^2 % M 可能的取值恰好会有 (M+1)/2 种; 
所以，装填因子小于 0.5 ，就可以保证找到空地址插入。

若M为素数，平方探测时， 从1,2... M/2, 这些值探测出来的位置肯定是不同的，后续则不一定。
所以，查找链要尽可能长，即M取更大的值。

相比于线性试探，
优点：很大程度缓解数据聚集现象，一旦冲突可以聪明地跳离是非之地；
缺点：由于跳的比较远，有可能导致对外存的IO，但这个影响一般不大；


关于外存IO的可能性：
假设page 是4k，一个hash地址空间是 4字节，那么 sqrt(4k / 4) = 32, 也就是说可以进行 32 次试探。
如果进行 32 次试探还不行，那真的是命中注定了。 

#### 开放地址 --- 双向平方探测
hask(key) = (key + 1^2) % M;
hask(key) = (key - 1^2) % M;
hask(key) = (key + 2^2) % M;
hask(key) = (key - 2^2) % M;

但是，M不仅要是素数，而且要满足 M = 4K + 3，这样可以保证查找链的前 M 项互异；

### 地址空间动态调整

关键在于装填因子，< 0.5

### 代码实现

template <typename K, typename V> //key、value
class Hashtable {
private:
   Entry<K, V>** ht; //桶数组，存放词条指针
   Bitmap* removed; //懒惰删除标记位图：总数L = removed->size() = removed->top
   Rank capacity;
   Rank size; //桶的总数、词条的数目：(size+L)/capacity <= Lambda_max

protected:
   Rank probe4Hit( const K& k ); //沿关键码k对应的试探链，找到词条匹配的桶
   Rank probe4Free( const K& k ); //沿关键码k对应的试探链，找到首个可用空桶
   void rehash(); //重散列算法：扩充桶数组，保证装填因子在警戒线以下

public:
   Hashtable( Rank c = 5 ); //创建一个容量不小于c的散列表（为测试暂时选用较小的默认值）
   ~Hashtable(); //释放桶数组及其中各（非空）元素所指向的词条
   Rank size() const { return size; }

   bool put( K, V ); //插入（禁止雷同词条，故可能失败）
   V* get( K k ); 
   bool remove( K k ); 

};

template <typename K, typename V> 
Hashtable <K, V>::Hashtable( Rank c ) {
   capacity = primeNLT( c, 1048576, "./prime-1048576-bitmap.txt" ); // 返回大于等于 c 的最小素数
   size = 0; 
   ht = new Entry<K, V>*[capacity]; 
   memset( ht, 0, sizeof( Entry<K, V>* ) * capacity ); 
   removed = new Bitmap( capacity ); //用位图记录懒惰删除位：总数 = removed->size() = removed->top
}

template <typename K, typename V> 
Hashtable<K, V>::~Hashtable() 
{ 
   for ( Rank i = 0; i < capacity; i++ ) 
      if ( ht[i] ) release( ht[i] );
   release( ht );
   release( removed );
} 

// key 值必须是唯一的
// 返回的 r：或者是 key 所在位置，或者是一个未装填过的位置
template <typename K, typename V> 
V* Hashtable<K, V>::get( K k ) 
{ 
	int r = probe4Hit ( k ); 
	return ht[r] ? &( ht[r]->value ) : NULL; 
}

// key 值必须唯一
// 懒惰删除标记过多，有可能会影响 查找性能
template <typename K, typename V> 
bool Hashtable<K, V>::put( K k, V v ) 
{
   if ( ht[ probe4Hit( k ) ] ) return false;
   Rank r = probe4Free( k ); //为新词条找个空桶（只要装填因子控制得当，必然成功）
   
   ht[ r ] = new Entry<K, V>( k, v ); 
   ++size; 
   if ( removed->test( r ) ) removed->clear( r );  //懒惰删除标记

   if ( (size + removed->size())*2 > capacity ) rehash(); //若装填因子高于50%，重散列
   return true;
}


// 这一步 rehash 存在疑问？？？比如反复插入、删除？？
template <typename K, typename V> 
bool Hashtable<K, V>::remove( K k ) 
{ 
   int r = probe4Hit( k );
   if ( !ht[r] ) return false; //确认目标词条确实存在

   release( ht[r] ); ht[r] = NULL; --size; 
   removed->set(r); 

   if ( removed->size() > 3*size ) rehash(); //若懒惰删除标记过多，重散列
   return true;
}


template <typename K, typename V> 
void Hashtable<K, V>::rehash() 
{
   Rank oldM = capacity; 
   Entry<K, V>** oldHt = ht;

   capacity = primeNLT( 4 * size, 1048576, PRIME_TABLE ); //容量至少加倍（若懒惰删除很多，可能反而缩容）
   size = 0; 
   ht = new Entry<K, V>*[capacity]; 
   memset( ht, 0, sizeof( Entry<K, V>* ) * capacity ); 
   release( removed ); 
   removed = new Bitmap( capacity ); //懒惰删除标记
   
   for ( Rank i = 0; i < oldM; i++ )
      if ( oldHt[i] ) 
         put( oldHt[i]->key, oldHt[i]->value ); //转入新表
   release( oldHt ); 
}


/******************************************************************************************
 * 沿关键码k的试探链，找到首个可用空桶；实践中有多种试探策略可选，这里仅以线性试探为例
 * 只要有空桶（无论是否带有懒惰删除标志）,迟早能找到
 ******************************************************************************************/
template <typename K, typename V> 
Rank Hashtable<K, V>::probe4Free( const K& k ) 
{
   for ( Rank r = hashCode( k ) % capacity;; r = ( r + 1 ) % capacity ) 
      if ( !ht[r] ) 
         return r; 
}


/******************************************************************************************
 * 沿关键码k的试探链，找到关键字 k；实践中有多种试探策略可选，这里仅以线性试探为例
 * 按除余法确定起点，线性试探
 * 办法：遇到同一个key，或者遇到一个空桶
 * 退出：只要 size+L < capacity，迟早能终止 --- 装填因子小于 0.5
 ******************************************************************************************/
template <typename K, typename V> 
Rank Hashtable<K, V>::probe4Hit( const K& k ) 
{
   for ( Rank r = hashCode( k ) % capacity;; r = ( r + 1 ) % capacity )
      if ( !ht[r] && !removed->test( r ) || ht[r] && ( k == ht[r]->key ) ) 
         return r; 
}