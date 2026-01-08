
/*

图 G = (V; E)
n = | V |, e = |E|;

v1 --- v2： v1 和 v2 之间存在邻接关系 adjacency
v1 --- e1:  v1 he e1 之间存在关联关系 incidence

无向边：一条边的两个顶尖之间，没有次序；
有向图：图中所有的边，都是有向的；  --- 通过有向图，可以表示无向图、混合图；
无向图：图中所有的边，都是无向的；
混合图：混合了有向、无向图；

路径 = (v0,v1,... vk)
路径长度 = K

一般路径：
简单路径：一条路径中，不含重复节点的路径；Vi = Vj 除非 i = j;

环路：v0=vk；
简单环路：没有重复的节点；


欧拉环路：经过图中的每条边一次；
汉密尔顿环路：经过图中的每个顶点一次；

图的表示：邻接矩阵，优点：直观、易理解，适用范围很广；缺点：空间O(n^2), 与实际边数无关，多数情况利用率很低；

以平面图为例证明：邻接矩阵的空间利用率很低
平面图：把G画在平面上，使得除了顶点外，任意2条边不交叉，称G为平面图；
在一个平面图中，e <= 3n -6 << O(n^2);  --- 平面欧拉公式
所以，空间利用率 <= (3n - 6) / n^2 = 3/n

BFS 复杂度: O(n + e), 用途：寻找最短路径；

DFS 复杂度：

括号定理：节点V的活跃期 active(v) = [dtime(v), ftime(v)],
u 是 v 的祖先，iff active(u) 包含 active(v);


*/


using VStatus = enum { UNDISCOVERED, DISCOVERED, VISITED }; //顶点状态
using EType = enum { UNDETERMINED, TREE, CROSS, FORWARD, BACKWARD }; //边在遍历树中所属的类型

template <typename Tv, typename Te>
class Graph { 
private:
   void reset() { //所有顶点、边的辅助信息复位
      for ( Rank v = 0; v < n; v++ ) { //所有顶点的
         status( v ) = UNDISCOVERED; dTime( v ) = fTime( v ) = -1; //状态，时间标签
         parent( v ) = -1; priority( v ) = INT_MAX; //（在遍历树中的）父节点，优先级数
         for ( Rank u = 0; u < n; u++ ) //所有边的
            if ( exists( v, u ) ) type( v, u ) = UNDETERMINED; //类型
      }
   }

   void BFS( Rank, Rank& ); //（连通域）广度优先搜索算法
   void DFS( Rank, Rank& ); //（连通域）深度优先搜索算法

   void BCC( Rank, Rank&, Stack<Rank>& ); //（连通域）基于DFS的双连通分量分解算法
   bool TSort( Rank, Rank&, Stack<Tv>* ); //（连通域）基于DFS的拓扑排序算法
   template <typename PU> void PFS( Rank, PU ); //（连通域）优先级搜索框架

public:
// 顶点
   Rank n; //顶点总数
   virtual Rank insert( Tv const& ) = 0; //插入顶点，返回编号
   virtual Tv remove( Rank ) = 0; //删除顶点及其关联边，返回该顶点信息
   virtual Tv& vertex( Rank ) = 0; //顶点的数据（该顶点的确存在）
   virtual Rank inDegree( Rank ) = 0; //顶点的入度（该顶点的确存在）
   virtual Rank outDegree( Rank ) = 0; //顶点的出度（该顶点的确存在）
   virtual Rank firstNbr( Rank ) = 0; //顶点的首个邻接顶点
   virtual Rank nextNbr( Rank, Rank ) = 0; //顶点（相对当前邻居的）下一邻居
   virtual VStatus& status( Rank ) = 0; //顶点的状态
   virtual Rank& dTime( Rank ) = 0; //顶点的时间标签dTime
   virtual Rank& fTime( Rank ) = 0; //顶点的时间标签fTime
   virtual Rank& parent( Rank ) = 0; //顶点在遍历树中的父亲
   virtual int& priority( Rank ) = 0; //顶点在遍历树中的优先级数
// 边：这里约定，无向边均统一转化为方向互逆的一对有向边，从而将无向图视作有向图的特例
   Rank e; //边总数
   virtual bool exists( Rank, Rank ) = 0; //边(v, u)是否存在
   virtual void insert( Te const&, int, Rank, Rank ) = 0; //在两个顶点之间插入指定权重的边
   virtual Te remove( Rank, Rank ) = 0; //删除一对顶点之间的边，返回该边信息
   virtual EType& type( Rank, Rank ) = 0; //边的类型
   virtual Te& edge( Rank, Rank ) = 0; //边的数据（该边的确存在）
   virtual int& weight( Rank, Rank ) = 0; //边(v, u)的权重
// 算法
   void bfs( Rank ); //广度优先搜索算法
   void dfs( Rank ); //深度优先搜索算法
   void bcc( Rank ); //基于DFS的双连通分量分解算法
   Stack<Tv>* tSort( Rank ); //基于DFS的拓扑排序算法
   void prim( Rank ); //最小支撑树Prim算法
   void dijkstra( Rank ); //最短路径Dijkstra算法
   template <typename PU> void pfs( Rank, PU ); //优先级搜索框架
};




template <typename Tv> struct Vertex 
{ 
   Tv data; 
   int inDegree, outDegree; 
   VStatus status; 
   Rank dTime, fTime; //时间标签，discover time, finish time
   Rank parent; 
   int priority;

   Vertex( Tv const& d = (Tv)0 ) : 
      data( d ), inDegree( 0 ), outDegree( 0 ), status( UNDISCOVERED ), dTime( -1 ),
      fTime( -1 ), parent( -1 ), priority( INT_MAX ) {} //暂不考虑权重溢出
};

template <typename Te> struct Edge {
   Te data; 
   int weight;
   EType type;

   Edge( Te const& d, int w ) : data( d ), weight( w ), type( UNDETERMINED ) {} 
};


//基于向量，以邻接矩阵形式实现的图
template <typename Tv, typename Te> 
class GraphMatrix : public Graph<Tv, Te> { 
   private:
      Vector< Vertex<Tv> > V; //顶点集（向量）
      Vector< Vector<Edge<Te>*> > E; //边集（邻接矩阵），二维矩阵

   public:
      GraphMatrix() { n = e = 0; } //构造
      ~GraphMatrix() 
      { 
         for ( Rank v = 0; v < n; v++ ) //所有动态创建的边记录
            for ( Rank u = 0; u < n; u++ ) 
               delete E[v][u]; //逐条清除
      }

   // 顶点的基本操作：查询第v个顶点（0 <= v < n）
      virtual Tv& vertex( Rank v ) { return V[v].data; } //数据
      virtual Rank inDegree( Rank v ) { return V[v].inDegree; } //入度
      virtual Rank outDegree( Rank v ) { return V[v].outDegree; } //出度
      virtual VStatus& status( Rank v ) { return V[v].status; } //状态
      virtual Rank& dTime( Rank v ) { return V[v].dTime; } //时间标签dTime
      virtual Rank& fTime( Rank v ) { return V[v].fTime; } //时间标签fTime
      virtual Rank& parent( Rank v ) { return V[v].parent; } //在遍历树中的父亲
      virtual int& priority( Rank v ) { return V[v].priority; } //在遍历树中的优先级数

      virtual Rank firstNbr( Rank v ) { return nextNbr( v, n ); } //首个邻接顶点
      virtual Rank nextNbr( Rank v, Rank u ) //相对于顶点u的下一邻接顶点（改用邻接表效率更高）
         { while ( ( -1 != --u ) && !exists( v, u ) ); return u; } //逆向线性试探

   // 边的确认操作
      virtual bool exists( Rank v, Rank u ) //边(v, u)是否存在
         { return ( v < n ) && ( u < n ) && ( E[v][u] != NULL ); }
   // 边的基本操作：查询顶点v与u之间的联边（0 <= v, u < n且exists(v, u)）
      virtual EType& type( Rank v, Rank u ) { return E[v][u]->type; } //边(v, u)的类型
      virtual Te& edge( Rank v, Rank u ) { return E[v][u]->data; } //边(v, u)的数据
      virtual int& weight( Rank v, Rank u ) { return E[v][u]->weight; } //边(v, u)的权重




   // 顶点的动态操作
      virtual Rank insert( Tv const& vertex ) 
      { //插入顶点，返回编号
         for ( Rank u = 0; u < n; u++ ) E[u].insert( NULL );  //各顶点预留一条潜在的关联边
         n++;
         E.insert( Vector<Edge<Te>*>( n, n, (Edge<Te>*)NULL ) ); //创建新顶点对应的边向量
         return V.insert( Vertex<Tv>( vertex ) ); //顶点向量增加一个顶点
      }
      virtual Tv remove( Rank v ) 
      { //删除第v个顶点及其关联边（0 <= v < n）
         for ( Rank u = 0; u < n; u++ ) //所有
            if ( exists( v, u ) ) //出边
               { delete E[v][u]; V[u].inDegree--; e--; } //逐条删除
         E.remove( v ); n--; //删除第v行

         Tv vBak = vertex( v ); V.remove( v ); //删除顶点v

         for ( Rank u = 0; u < n; u++ ) //所有
            if ( Edge<Te>* x = E[u].remove( v ) ) //入边
               { delete x; V[u].outDegree--; e--; } //逐条删除

         return vBak; //返回被删除顶点的信息
      }

   // 边的动态操作
      virtual void insert( Te const& edge, int w, Rank v, Rank u ) { //插入权重为w的边(v, u)
         if ( exists( v, u ) ) return; //确保该边尚不存在
         E[v][u] = new Edge<Te>( edge, w ); //创建新边
         e++; 
         V[v].outDegree++; 
         V[u].inDegree++; //更新边计数与关联顶点的度数
      }

      virtual Te remove( Rank v, Rank u ) { //删除顶点v和u之间的联边（exists(v, u)）
         Te eBak = edge( v, u ); 
         delete E[v][u];
         E[v][u] = NULL; //备份后删除边记录
         e--; 
         V[v].outDegree--;
         V[u].inDegree--; //更新边计数与关联顶点的度数
         return eBak; //返回被删除边的信息
      }
}; 



//广度优先搜索BFS算法（全图）
//如此可完整覆盖全图，且总体复杂度依然保持为O(n+e)
template <typename Tv, typename Te> 
void Graph<Tv, Te>::bfs( Rank s ) 
{ // s < n
   reset(); 
   Rank dClock = 0; //全图复位
   for ( Rank v = s; v < s + n; v++ ) //从s起顺次检查所有顶点
      if ( UNDISCOVERED == status( v % n ) ) //一旦遇到尚未发现者
         BFS( v % n, dClock ); //即从它出发启动一次BFS
} 

//广度优先搜索BFS算法（单个连通域）
template <typename Tv, typename Te> 
void Graph<Tv, Te>::BFS( Rank v, Rank& dClock ) 
{ // v < n
   Queue<Rank> Q; 
   status( v ) = DISCOVERED; 
   Q.enqueue( v ); 
   dTime( v ) = dClock;
   dClock++;

   for ( Rank fClock = 0; !Q.empty(); ) 
   {
      if ( dTime( v ) < dTime( Q.front() ) ) //dTime的增加，意味着开启新的一代，因此
         dClock++, fClock = 0; //dTime递增，fTime复位
      
      v = Q.dequeue(); 
      for ( Rank u = firstNbr( v ); -1 != u; u = nextNbr( v, u ) ) //考查v的每一个邻居u
         if ( UNDISCOVERED == status( u ) ) { //若u尚未被发现，则发现之
            status( u ) = DISCOVERED; 
            Q.enqueue( u ); 
            dTime( u ) = dClock;   // 同一批 dTime 相同
            type( v, u ) = TREE; 
            parent( u ) = v; //引入树边，拓展BFS树
         } else //若u已被发现，或者甚至已访问完毕，则
            type( v, u ) = CROSS; //将(v, u)归类于跨边

      status( v ) = VISITED; 
      fTime( v ) = fClock++; //至此，v访问完毕
   } 
} 



//深度优先搜索DFS算法（全图）
//如此可完整覆盖全图，且总体复杂度依然保持为O(n+e)
template <typename Tv, typename Te> 
void Graph<Tv, Te>::dfs( Rank s ) 
{ // s < n
   reset(); 
   Rank clock = 0; //全图复位
   for ( Rank v = s; v < s + n; v++ ) //从s起顺次检查所有顶点
      if ( UNDISCOVERED == status( v % n ) ) //一旦遇到尚未发现者
         DFS( v % n, clock ); //即从它出发启动一次DFS
} 

//深度优先搜索DFS算法（单个连通域）
// 递归实现
template <typename Tv, typename Te> 
void Graph<Tv, Te>::DFS( Rank v, Rank& clock ) 
{ // v < n
   dTime( v ) = clock++; 
   status( v ) = DISCOVERED; //发现当前顶点v

   for ( Rank u = firstNbr( v ); - 1 != u; u = nextNbr( v, u ) ) //考查v的每一个邻居u
      switch ( status( u ) ) { //并视其状态分别处理
         case UNDISCOVERED : // u尚未发现，意味着支撑树可在此拓展
            type( v, u ) = TREE; parent( u ) = v; DFS( u, clock ); break;
         case DISCOVERED : // u已被发现但尚未访问完毕，应属被后代指向的祖先
            type( v, u ) = BACKWARD; break;
         default : // u已访问完毕（VISITED，有向图），则视承袭关系分为前向边或跨边
            type( v, u ) = ( dTime( v ) < dTime( u ) ) ? FORWARD : CROSS; break;
      }

   status( v ) = VISITED; 
   fTime( v ) = clock++; 
}
