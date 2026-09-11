
/*
	红黑树定义：
	1. 树根：黑色；
	2. 外部节点：都为黑色；
	3. 外部节点到根节点：途中黑色节点数目相同；
	4. 红色节点：父亲、孩子都是黑色；

	提升变换：把红节点提升到和其父节点同一层次，就成了 (2,4)树。注意到，这时候所有的叶节点高度一样了。

	所以，红黑树就是4阶B树。
	--- 黑 ---
	|        |
	黑       黑

	--- 黑-红 ---
	|     |     |
	黑    黑    黑	

	--- 红-黑 ---
	|     |     |
	黑    黑    黑	

	--- 红-黑 - 红---
	|     |  |      |
	黑    黑  黑     黑	


	插入一个节点，并把这个节点染色为红色（为什么是红色？因为这样符合条件123）。
	如果父亲是黑色，则没有问题；
	如果父亲是红色，那么要解决双红的问题；

	参见：algo_202402122025.png , algo_202402122030.png


*/



#include "tree.h" //基于BST实现RedBlack
template <typename T> class RedBlack : public BST<T> 
{
protected:
   void solveDoubleRed( BinNodePosi<T> x ); //双红修正
   void solveDoubleBlack( BinNodePosi<T> x ); //双黑修正
   Rank updateHeight( BinNodePosi<T> x ); //更新节点x的高度（重写）
public:
   BinNodePosi<T> insert( const T& e ); //插入（重写）
   bool remove( const T& e ); //删除（重写）
// BST::search()等其余接口可直接沿用
};

template <typename T> 
Rank RedBlack<T>::updateHeight( BinNodePosi<T> x ) {
	return x->height = IsBlack( x ) + max( stature( x->lc ), stature( x->rc ) );
}

template <typename T> 
BinNodePosi<T> RedBlack<T>::insert( const T& e ) 
{
	BinNodePosi<T>& x = search( e ); 
	if ( x ) return x;

	x = new BinNode<T>( e, _hot, NULL, NULL, 0 ); 
	_size++;
	BinNodePosi<T> xOld = x; 
	solveDoubleRed( x ); 
	return xOld;
}

/******************************************************************************************
 * RedBlack双红调整算法：解决节点x与其父均为红色的问题。分为两大类情况：
 *    RR-1：2次颜色翻转，2次黑高度更新，1~2次旋转，不再递归
 *    RR-2：3次颜色翻转，3次黑高度更新，0次旋转，需要递归
 ******************************************************************************************/
template <typename T> 
void RedBlack<T>::solveDoubleRed( BinNodePosi<T> x ) { // x当前必为红
   if ( IsRoot( *x ) ) //若已（递归）转至树根，则将其转黑，整树黑高度也随之递增
    {  _root->color = RB_BLACK; _root->height++; return;  } //否则，x的父亲p必存在

   BinNodePosi<T> p = x->parent; if ( IsBlack( p ) ) return; //若p为黑，则可终止调整。

   BinNodePosi<T> g = p->parent; //既然p为红，则x的祖父必存在，且必为黑色
   BinNodePosi<T> u = uncle( x ); //以下，视x叔父u的颜色分别处理
   if ( IsBlack( u ) ) { // u为黑色（含NULL）时
      if ( IsLChild( *x ) == IsLChild( *p ) ) //若x与p同侧（即zIg-zIg或zAg-zAg），则
         p->color = RB_BLACK; //p由红转黑，x保持红
      else //若x与p异侧（即zIg-zAg或zAg-zIg），则
         x->color = RB_BLACK; //x由红转黑，p保持红
      g->color = RB_RED; //g必定由黑转红
///// 以上虽保证总共两次染色，但因增加了判断而得不偿失
///// 在旋转后将根置黑、孩子置红，虽需三次染色但效率更高
      BinNodePosi<T> gg = g->parent; //曾祖父（great-grand parent）
      BinNodePosi<T> r = FromParentTo( *g ) = rotateAt( x ); //调整后的子树根节点
      r->parent = gg; //与原曾祖父联接
   } else { //若u为红色
      p->color = RB_BLACK; p->height++; //p由红转黑
      u->color = RB_BLACK; u->height++; //u由红转黑
      g->color = RB_RED; //在B-树中g相当于上交给父节点的关键码，故暂标记为红
      solveDoubleRed( g ); //继续调整：若已至树根，接下来的递归会将g转黑（尾递归，不难改为迭代）
   }
}