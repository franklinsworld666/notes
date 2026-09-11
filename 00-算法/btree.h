
/*

cpu访问外存的2个事实：
1. 访问外存是时间，是内存的 10^5 倍以上；
2. 访问外存 1字节与1KB，时间一样；

所以，设计了B树（每一个节点尽可能有很多关键值，可以减少对外存的访问次数）；

时间复杂度：log(n),但是在常系数上，B树比一般的BST要小很多(高度更低）。

一个 m 阶的B树： --- m 路平衡搜索树
1. 每一个内部节点：       不超过 (m-1) 个 key；
2. 树根的分支数：         2 <=  X <= m;
3. 其余内部节点的分支数:   m/2向上取整 <= X <= m;
4. 所有外部节点的深度统一相等；

所以，对于这样一颗 m 阶树，也称为：(m/2, m)树；

由以上这些条件，可以推出 B 树的高度范围：最小树高：O(logmN)

如果一颗二叉树与256阶树相比，大约高度是 log2N / log256N = 8
*/




#include "vector.h"
template <typename T> struct BTNode;
template <typename T> using BTNodePosi = BTNode<T>*;

template <typename T> struct BTNode 
{
   BTNodePosi<T> parent; 
   Vector<T> key;
   Vector<BTNodePosi<T>> child; //孩子向量（总比关键码多一个）


   BTNode() { parent = NULL; child.insert( NULL ); } //无关键码节点
   BTNode( T e, BTNodePosi<T> lc = NULL, BTNodePosi<T> rc = NULL ) 
   {
      parent = NULL; key.insert( e ); //作为根节点只有一个关键码，以及左右孩子
      child.insert( lc ); if ( lc ) lc->parent = this; 
      child.insert( rc ); if ( rc ) rc->parent = this; 
   }
   ~BTNode() {// recursive release all child}
};

template <typename T> class BTree 
{
protected:
   Rank _size; //存放的关键码总数
   Rank _m; // B-树的阶次，至少为3——创建时指定，一般不能修改
   BTNodePosi<T> _root; //根节点
   BTNodePosi<T> _hot; // BTree::search()最后访问的非空（除非树空）的节点位置


   void solveOverflow( BTNodePosi<T> ); //因插入导致分支增加而上溢之后的分裂处理
   void solveUnderflow( BTNodePosi<T> ); //因删除导致分支减少而下溢之后的合并处理

public:
   BTree( int m = 3 ) : _m( m ), _size( 0 ) //构造函数：默认为最低的3阶
      { _root = new BTNode<T>(); }
   ~BTree() { if ( _root ) release( _root ); } 

   int const order() { return _m; } 
   int const size() { return _size; } 
   BTNodePosi<T> & root() { return _root; } 
   bool empty() const { return !_root; } 

   BTNodePosi<T> search ( const T& e ); 
   bool insert ( const T& e ); 
   bool remove ( const T& e ); 
}; 

// 关键码，如果超过了m-1，这个节点从中间分裂成2个节点，中间的节点上移
// k0...k(s-1),  ks, k(s+1)...km
void BTree<T>::solveOverflow( BTNodePosi<T> v)
{
	while ( _m <= v->key.size() ) {
		Rank s = _m/2;
      	
		BTNodePosi<T> u = new BTNode<T>(); //注意：新节点已有一个空孩子
      	for ( Rank j = 0; j < _m - s - 1; j++ ) { // v右侧_m-s-1个孩子及关键码分裂为右侧节点u
        	u->child.insert( j, v->child.remove( s + 1 ) ); //逐个移动效率低
        	u->key.insert( j, v->key.remove( s + 1 ) ); //此策略可改进
      	}		
		u->key.insert(_m-s-1, v->key.remove(s+1));
		u->child[_m - s - 1] = v->child.remove( s + 1 );

		if (u->child[0])  // 非叶节点
			for ( Rank j = 0; j < _m - s; j++ ) //令它们的父节点统一
				u->child[j]->parent = u;

		BTNodePosi<T> p = v->parent;
      	if ( !p ) { _root = p = new BTNode<T>(); p->child[0] = v; v->parent = p; } 
      	
		Rank r = 1 + p->key.search( v->key[s] ); // p中指向v的指针的秩
		p->key.insert( r, v->key.remove( s ) ); //轴点关键码上升
      	p->child.insert( r + 1, u ); 
		u->parent = p; //新节点u与父节点p互联
      
	  	v = p; 	
	}
}

// 从左或右中借
// 与左或右合并
void BTree<T>::solveUnderflow( BTNodePosi<T> v)
{
   while ( ( _m + 1 ) / 2 > v->child.size() ) {
      BTNodePosi<T> p = v->parent;
      if ( !p ) { //已到根节点（两个孩子即可）
         if ( !v->key.size() && v->child[0] ) {
            //但倘若作为树根的v已不含关键码，却有（唯一的）非空孩子，则
            /*DSA*/ printf( "collapse\n" ); 
            _root = v->child[0]; _root->parent = NULL; //这个节点可被跳过
            v->child[0] = NULL; release( v ); //并因不再有用而被销毁
         } //整树高度降低一层
         return;
      }
	  
      Rank r = 0;
      while ( p->child[r] != v ) r++;
      //确定v是p的第r个孩子——此时v可能不含关键码，故不能通过关键码查找
      //另外，在实现了孩子指针的判等器之后，也可直接调用Vector::find()定位
      /*DSA*/ printf( "\nrank = %d", r );
   // 情况1：向左兄弟借关键码
      if ( 0 < r ) { //若v不是p的第一个孩子，则
         BTNodePosi<T> ls = p->child[r - 1]; //左兄弟必存在
         if ( ( _m + 1 ) / 2 < ls->child.size() ) { //若该兄弟足够“胖”，则
            /*DSA*/ printf( " ... case 1\n" );
            v->key.insert( 0, p->key[r - 1] ); // p借出一个关键码给v（作为最小关键码）
            p->key[r - 1] = ls->key.remove( ls->key.size() - 1 ); // ls的最大关键码转入p
            v->child.insert( 0, ls->child.remove( ls->child.size() - 1 ) );
            //同时ls的最右侧孩子过继给v
            if ( v->child[0] ) v->child[0]->parent = v; //作为v的最左侧孩子
            return; //至此，通过右旋已完成当前层（以及所有层）的下溢处理
         }
      } //至此，左兄弟要么为空，要么太“瘦”
   // 情况2：向右兄弟借关键码
      if ( p->child.size() - 1 > r ) { //若v不是p的最后一个孩子，则
         BTNodePosi<T> rs = p->child[r + 1]; //右兄弟必存在
         if ( ( _m + 1 ) / 2 < rs->child.size() ) { //若该兄弟足够“胖”，则
            /*DSA*/ printf( " ... case 2\n" );
            v->key.insert( v->key.size(),
                           p->key[r] ); // p借出一个关键码给v（作为最大关键码）
            p->key[r] = rs->key.remove( 0 ); // rs的最小关键码转入p
            v->child.insert( v->child.size(), rs->child.remove( 0 ) );
            //同时rs的最左侧孩子过继给v
            if ( v->child[v->child.size() - 1] ) //作为v的最右侧孩子
               v->child[v->child.size() - 1]->parent = v;
            return; //至此，通过左旋已完成当前层（以及所有层）的下溢处理
         }
      } //至此，右兄弟要么为空，要么太“瘦”
   // 情况3：左、右兄弟要么为空（但不可能同时），要么都太“瘦”——合并
      if ( 0 < r ) { //与左兄弟合并
         /*DSA*/printf ( " ... case 3L\n" );
         BTNodePosi<T> ls = p->child[r - 1]; //左兄弟必存在
         ls->key.insert( ls->key.size(), p->key.remove( r - 1 ) ); p->child.remove( r );
         // p的第r - 1个关键码转入ls，v不再是p的第r个孩子
         ls->child.insert( ls->child.size(), v->child.remove( 0 ) );
         if ( ls->child[ls->child.size() - 1] ) // v的最左侧孩子过继给ls做最右侧孩子
            ls->child[ls->child.size() - 1]->parent = ls;
         while ( !v->key.empty() ) { // v剩余的关键码和孩子，依次转入ls
            ls->key.insert( ls->key.size(), v->key.remove( 0 ) );
            ls->child.insert( ls->child.size(), v->child.remove( 0 ) );
            if ( ls->child[ls->child.size() - 1] ) 
               ls->child[ls->child.size() - 1]->parent = ls;
         }
         release ( v ); //释放v
      } else { //与右兄弟合并
         /*DSA*/ printf( " ... case 3R\n" );
         BTNodePosi<T> rs = p->child[r + 1]; //右兄弟必存在
         rs->key.insert( 0, p->key.remove( r ) ); p->child.remove( r );
         // p的第r个关键码转入rs，v不再是p的第r个孩子
         rs->child.insert( 0, v->child.remove( v->child.size() - 1 ) );
         if ( rs->child[0] ) rs->child[0]->parent = rs; // v的最右侧孩子过继给rs做最左侧孩子
         while ( !v->key.empty() ) { // v剩余的关键码和孩子，依次转入rs
            rs->key.insert( 0, v->key.remove( v->key.size() - 1 ) );
            rs->child.insert( 0, v->child.remove( v->child.size() - 1 ) );
            if ( rs->child[0] ) rs->child[0]->parent = rs;
         }
         release( v ); //释放v
      }
      v = p; //上升一层，如有必要则继续旋转或合并——至多O(logn)层
   }
}


// 存在 返回 节点；不存在，返回NULL
// _hot :指向最后一个访问的节点 的父节点
BTNodePosi<T> BTree<T>::search(const T& e)
{
	BTNodePosi<T> v = _root;
	_hot = NULL;
	while (v) {
		// 在当前节点 v 的 vector 中查找 e
		Rank r = v->key.search(e);
		if (r >= 0 && v->key[r] == e) return v;
		_hot = v;
		// 转到下一层的分支
		v = v->child[r+1];
	}
	return NULL;
}

bool BTree<T>::insert(const T& e)
{
	BTNodePosi<T> v = BTree<T>::search(e);
	if (v) return false;

	if (!_hot) {
		_root = new BTNode(e);
		_size = 1;
		return true;
	}

	Rank r = _hot->key.search(e);
	_hot->key.insert(r+1, e);
	_hot->child.insert(NULL);
	_size++;
	solveOverflow( _hot );

	return true;
}

// 找到待删除节点后，与 后继节点 交换
// 在B树种，这个后继节点一定位于 叶节点最左边的一个元素
bool BTree<T>::remove(const T& e)
{
	BTNodePosi<T> v = BTree<T>::search(e);
	if (!v) return false;

	// find e
	Rank r = v->key.search( e );
	if (v->child[0]) {
		BTNodePosi<T> u = v->child[r+1];
		while (u && u->child[0]) u = u->child[0];
		
		v->key[r] = u->key[0];
		v = u;
		r = 0;
	}
	v->key.remove(r);
	v->child.remove(r+1);
	_size--;
	
	solveUnderflow(v);
	
	return true;
}

// 从中位数 key 拿出来，插入父节点中
//  当前节点分成2个节点，分别是上插节点的左右孩子
void BTree<T>::solveOverflow( BTNodePosi<T> v)
{
	// 当前节点上溢
   while ( _m <= v->key.size() ) 
   { 
      Rank s = _m / 2; //轴点（此时应有_m = key.size() = child.size() - 1）
      BTNodePosi<T> u = new BTNode<T>(); //注意：新节点已有一个空孩子

		// v右侧_m-s-1个孩子及关键码分裂为右侧节点u
      for ( Rank j = 0; j < _m - s - 1; j++ ) 
	  { 
         u->child.insert( j, v->child.remove( s + 1 ) ); //逐个移动效率低
         u->key.insert( j, v->key.remove( s + 1 ) ); //此策略可改进
      }
      u->child[_m - s - 1] = v->child.remove( s + 1 ); //移动v最靠右的孩子
      if ( u->child[0] ) //若u的孩子们非空，则
         for ( Rank j = 0; j < _m - s; j++ ) //令它们的父节点统一
            u->child[j]->parent = u; //指向u

      BTNodePosi<T> p = v->parent; // v当前的父节点p
      if ( !p ) { _root = p = new BTNode<T>(); p->child[0] = v; v->parent = p; } //若p空则创建之
      Rank r = 1 + p->key.search( v->key[0] ); // p中指向v的指针的秩
      p->key.insert( r, v->key.remove( s ) ); //轴点关键码上升
      p->child.insert( r + 1, u ); u->parent = p; //新节点u与父节点p互联

      v = p; //上升一层，如有必要则继续分裂——至多O(logn)层
   } 
}


void BTree<T>::solveUnderflow( BTNodePosi<T> )
{
   while ( ( _m + 1 ) / 2 > v->child.size() ) { //除非当前节点并未下溢
      BTNodePosi<T> p = v->parent;
      if ( !p ) { //已到根节点（两个孩子即可）
         if ( !v->key.size() && v->child[0] ) {
            //但倘若作为树根的v已不含关键码，却有（唯一的）非空孩子，则
            /*DSA*/ printf( "collapse\n" ); 
            _root = v->child[0]; _root->parent = NULL; //这个节点可被跳过
            v->child[0] = NULL; release( v ); //并因不再有用而被销毁
         } //整树高度降低一层
         return;
      }
      Rank r = 0;
      while ( p->child[r] != v ) r++;
      //确定v是p的第r个孩子——此时v可能不含关键码，故不能通过关键码查找
      //另外，在实现了孩子指针的判等器之后，也可直接调用Vector::find()定位
      /*DSA*/ printf( "\nrank = %d", r );
   // 情况1：向左兄弟借关键码
      if ( 0 < r ) { //若v不是p的第一个孩子，则
         BTNodePosi<T> ls = p->child[r - 1]; //左兄弟必存在
         if ( ( _m + 1 ) / 2 < ls->child.size() ) { //若该兄弟足够“胖”，则
            /*DSA*/ printf( " ... case 1\n" );
            v->key.insert( 0, p->key[r - 1] ); // p借出一个关键码给v（作为最小关键码）
            p->key[r - 1] = ls->key.remove( ls->key.size() - 1 ); // ls的最大关键码转入p
            v->child.insert( 0, ls->child.remove( ls->child.size() - 1 ) );
            //同时ls的最右侧孩子过继给v
            if ( v->child[0] ) v->child[0]->parent = v; //作为v的最左侧孩子
            return; //至此，通过右旋已完成当前层（以及所有层）的下溢处理
         }
      } //至此，左兄弟要么为空，要么太“瘦”
   // 情况2：向右兄弟借关键码
      if ( p->child.size() - 1 > r ) { //若v不是p的最后一个孩子，则
         BTNodePosi<T> rs = p->child[r + 1]; //右兄弟必存在
         if ( ( _m + 1 ) / 2 < rs->child.size() ) { //若该兄弟足够“胖”，则
            /*DSA*/ printf( " ... case 2\n" );
            v->key.insert( v->key.size(),
                           p->key[r] ); // p借出一个关键码给v（作为最大关键码）
            p->key[r] = rs->key.remove( 0 ); // rs的最小关键码转入p
            v->child.insert( v->child.size(), rs->child.remove( 0 ) );
            //同时rs的最左侧孩子过继给v
            if ( v->child[v->child.size() - 1] ) //作为v的最右侧孩子
               v->child[v->child.size() - 1]->parent = v;
            return; //至此，通过左旋已完成当前层（以及所有层）的下溢处理
         }
      } //至此，右兄弟要么为空，要么太“瘦”
   // 情况3：左、右兄弟要么为空（但不可能同时），要么都太“瘦”——合并
      if ( 0 < r ) { //与左兄弟合并
         /*DSA*/printf ( " ... case 3L\n" );
         BTNodePosi<T> ls = p->child[r - 1]; //左兄弟必存在
         ls->key.insert( ls->key.size(), p->key.remove( r - 1 ) ); p->child.remove( r );
         // p的第r - 1个关键码转入ls，v不再是p的第r个孩子
         ls->child.insert( ls->child.size(), v->child.remove( 0 ) );
         if ( ls->child[ls->child.size() - 1] ) // v的最左侧孩子过继给ls做最右侧孩子
            ls->child[ls->child.size() - 1]->parent = ls;
         while ( !v->key.empty() ) { // v剩余的关键码和孩子，依次转入ls
            ls->key.insert( ls->key.size(), v->key.remove( 0 ) );
            ls->child.insert( ls->child.size(), v->child.remove( 0 ) );
            if ( ls->child[ls->child.size() - 1] ) 
               ls->child[ls->child.size() - 1]->parent = ls;
         }
         release ( v ); //释放v
      } else { //与右兄弟合并
         /*DSA*/ printf( " ... case 3R\n" );
         BTNodePosi<T> rs = p->child[r + 1]; //右兄弟必存在
         rs->key.insert( 0, p->key.remove( r ) ); p->child.remove( r );
         // p的第r个关键码转入rs，v不再是p的第r个孩子
         rs->child.insert( 0, v->child.remove( v->child.size() - 1 ) );
         if ( rs->child[0] ) rs->child[0]->parent = rs; // v的最右侧孩子过继给rs做最左侧孩子
         while ( !v->key.empty() ) { // v剩余的关键码和孩子，依次转入rs
            rs->key.insert( 0, v->key.remove( v->key.size() - 1 ) );
            rs->child.insert( 0, v->child.remove( v->child.size() - 1 ) );
            if ( rs->child[0] ) rs->child[0]->parent = rs;
         }
         release( v ); //释放v
      }
      v = p; //上升一层，如有必要则继续旋转或合并——至多O(logn)层
   } //while
}