
/*

伸展策略：
1. 同侧情况，先调整祖父节点，再调整父亲节点
    |
--- g ---
|	     |
T0	  ---p ----
      |        |
      T1   T2--V  -- T3

调整后：
    			|
		   ---- v ------
           |	        |
	    ---p ----       T3
        |       |
    T0--g--T1   T2


2. 异侧情况，先调整父节点，再调整子节点
    |
--- g ----
|	      |
T0	   ---p ----
       |        |
   T1--V--T3    T4


调整后：
    			|
	     ------ v ------
	     |              |
	  ---g---        ---p--- 
      |      |       |      |
     T0      T1      T2    T3



分摊复杂度:O(logn);

在 k << n << m 时，局部性会非常强。任意连续的 m 次查找，时间复杂度O(mlogk+nlogn), 
于是分摊复杂度为 O(logk);

但是无法保证单次最坏情况的出现，无法用于对效率十分敏感的场合；
*/

#include "tree.h"

template <typename T> class BST : public BinTree<T> {
protected:
   BinNodePosi<T> _hot; //“命中”节点的父亲

   BinNodePosi<T> connect34 ( //按照“3 + 4”结构，联接3个节点及四棵子树
   BinNodePosi<T>, BinNodePosi<T>, BinNodePosi<T>,
   BinNodePosi<T>, BinNodePosi<T>, BinNodePosi<T>, BinNodePosi<T> );

   BinNodePosi<T> rotateAt ( BinNodePosi<T> x ); //对x及其父亲、祖父做统一旋转调整

public: //基本接口：以virtual修饰，强制要求所有派生类（BST变种）根据各自的规则对其重写
   virtual BinNodePosi<T> & search ( const T& e ); //查找
   virtual BinNodePosi<T> insert ( const T& e ); //插入
   virtual bool remove ( const T& e ); //删除

};

// 查找 e，返回命中节点，_hot 为命中节点父亲；
// 或者 返回空，_hot 为最后比较的节点
template <typename T> 
BinNodePosi<T>& BST<T>::search( const T& e )
{
	BinNodePosi<T> v = _root;
	_hot = NULL;
	while (v) {
		if (v->data == e) {_hot = v->parent; return v;}
		_hot = v;
		v = (e < v->data ? v->lc : v->rc);
	}
	return NULL;


}


template <typename NodePosi> inline //在节点*p与*lc（可能为空）之间建立父（左）子关系
void attachAsLC( NodePosi lc, NodePosi p ) { p->lc = lc; if ( lc ) lc->parent = p; }

template <typename NodePosi> inline //在节点*p与*rc（可能为空）之间建立父（右）子关系
void attachAsRC( NodePosi p, NodePosi rc ) { p->rc = rc; if ( rc ) rc->parent = p; }


//由BST派生的Splay树模板类
template <typename T> 
class Splay : public BST<T> { 
protected:
	//将节点 v 伸展至根
   BinNodePosi<T> splayAlgo( BinNodePosi<T> v ); 

public:
   BinNodePosi<T>& search( const T& e ); //查找（重写）
   BinNodePosi<T> insert( const T& e ); //插入（重写）
   bool remove( const T& e ); //删除（重写）
};

template <typename T> 
BinNodePosi<T> Splay<T>::splayAlgo( BinNodePosi<T> v )
{
	BinNodePosi<T> p,g,gg;
	while ( (p = v->parent) && (g = p->parent) ) {
		gg = g->parent;
		if (IsLChild(*v)) {
			if (IsLChild(*p)) {
				
			} else {

			}
		} else {
			if (IsRChild(*p)) {
				attachAsRC(g, p->lc);
				attachAsLC(g, p);
				attachAsRC(p, v->lc);
				attachAsLC(p, v);
			} else {
				
			}
		}
		if (!gg) v->parent = NULL;
		else if (gg->lc == g) gg->lc = v;
		else gg->rc = v;

		updateHeight(g);updateHeight(p);updateHeight(v);
	}
	if ((p = v->parent) && IsLChild(*v)) {
		attachAsLC(v->rc, p);
		attachAsRC(v, p);
		v->parent = NULL;
		updateHeightAbove(p);
		_root = v;
	} else if (p = v->parent) {
		attachAsRC(p, v->lc);
		attachAsLC(p, v);
		v->parent = NULL;
		updateHeightAbove(p);
		_root = v;
	}
	return _root;
}


template <typename T> 
BinNodePosi<T> Splay<T>::insert( const T& e)
{
	// 空树
	BinNodePosi<T> v = search(e);
	if (!v) {
		_root = new BinNode<T>( e );
		_size = 1;
		return _root;
	}
	
	// 已经存在
	if (e == v->data) return v;

	if (e < v->data) {
		BinNodePosi<T> p = new BinNode<T>(e, NULL, v->lc, v);	
		v->parent = p;
		if (v->lc) {v->lc->parent = p; v->lc = NULL;}
	}
	else {
		BinNodePosi<T> p = new BinNode<T>(e, NULL, v, v->rc);	
		v->parent = p;
		if (v->rc) {v->rc->parent = p; v->rc = NULL;}
	}
	_root = p;
	_size++;
	updateHeightAbove(v);

	return _root;
}


template <typename T> 
bool Splay<T>::remove( const T& e )
{
   if ( !_root || ( e != search( e )->data ) ) 
   		return false;

   BinNodePosi<T> L = _root->lc, R = _root->rc; release( _root ); 
   if ( !R ) { //若R空，
      if ( L ) L->parent = NULL; 
	  _root = L; //L即是余树
   } else {
	  //在R中再次查找e：注定失败，但其中的最小节点必
	  //伸展至根（且无左孩子），故可令其以L作为左子树
      _root = R; R->parent = NULL; search( e ); 
      if (L) L->parent = _root; _root->lc = L; 
   }
   if ( --_size ) 
   		updateHeight( _root ); 
   return true; 
}

template <typename T> 
BinNodePosi<T>& Splay<T>::search( const T& e )
{
	BinNodePosi<T> v = BST<T>::search(e);
   _root = splayAlgo( v ? v : _hot); 
   return _root;
}


