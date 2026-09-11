#ifndef Y_LIST_H
#define Y_LIST_H

typedef unsigned int Rank;

template <typename T> struct ListNode;
template <typename T> using ListNodePosi = ListNode<T>*; 

template <typename T> struct ListNode 
{ 
   T data; 
   ListNodePosi<T> pred;
   ListNodePosi<T> succ;

   ListNode() {pred = succ = NULL;} 
   ListNode (T e, ListNodePosi<T> p = NULL, ListNodePosi<T> s = NULL )
      : data( e ), pred( p ), succ( s ) {} 

// 操作接口
   ListNodePosi<T> insertAsPred( T const& e ); 
   ListNodePosi<T> insertAsSucc( T const& e ); 
};

//紧靠当前节点之前插入新节点
template <typename T>
ListNodePosi<T> ListNode<T>::insertAsPred( T const& e ) 
{
	ListNodePosi<T> cur = new ListNode(e);
	cur->succ = this;
	cur->pred = this->pred;
	this->pred->succ = cur;
	this->pred = cur;
	return cur;
}


//紧随当前节点之后插入新节点
template <typename T>
ListNodePosi<T> ListNode<T>::insertAsSucc( T const& e ) 
{
	ListNodePosi<T> cur = new ListNode(e);
	cur->succ = this->succ;
	cur->pred = this;
	this->succ->pred = cur;
	this->succ = cur;
	return cur;
}


template <typename T> 
class List 
{
private:
	Rank _size; 
	ListNodePosi<T> header;
	ListNodePosi<T> trailer;

protected:
	void init(); //列表创建时的初始化
	void copyNodes( ListNodePosi<T>, Rank ); //复制列表中自位置p起的n项
	Rank clear(); //清除所有节点

	ListNodePosi<T> merge( ListNodePosi<T>, Rank, List<T>&, ListNodePosi<T>, Rank ); //归并
	void mergeSort( ListNodePosi<T>&, Rank ); //对从p开始连续的n个节点归并排序
	void selectionSort( ListNodePosi<T>, Rank ); //对从p开始连续的n个节点选择排序
	void insertionSort( ListNodePosi<T>, Rank ); //对从p开始连续的n个节点插入排序
	void radixSort( ListNodePosi<T>, Rank ); //对从p开始连续的n个节点基数排序

public:
	List() { init(); } 
	List( List<T> const& L ) { copyNodes( L.first(), L._size); }
	~List() {clear();delete header;delete trailer;}

	Rank size() const { return _size; }
	bool empty() const { return _size <= 0; }
	ListNodePosi<T> first() const { return header->succ; }
	ListNodePosi<T> last() const { return trailer->pred; }
	bool valid( ListNodePosi<T> p )
	{ return p && ( trailer != p ) && ( header != p ); }


	ListNodePosi<T> insert( ListNodePosi<T> p, T const& e ); //将e当作p的后继插入
	ListNodePosi<T> insert( T const& e, ListNodePosi<T> p ); //将e当作p的前驱插入
	
	T remove( ListNodePosi<T> p ); //删除合法位置p处的节点,返回被删除节点


	ListNodePosi<T> find( T const& e ) const //无序列表查找
	{ return find( e, _size, trailer ); }
	ListNodePosi<T> find( T const& e, Rank n, ListNodePosi<T> p ) const; //无序区间查找


	ListNodePosi<T> search( T const& e ) const //有序列表查找
	{ return search( e, _size, trailer ); }
	ListNodePosi<T> search( T const& e, Rank n, ListNodePosi<T> p ) const; //有序区间查找


	void merge( List<T>& L ) { merge( header->succ, _size, L, L.header->succ, L._size ); } //全列表归并
	void sort( ListNodePosi<T>, Rank ); //列表区间排序
	void sort() { sort( first(), _size ); } //列表整体排序
}; 

template <typename T>
void List<T>::init()
{
	_size = 0;
	header = new ListNode<T>();
	trailer = new ListNode<T>();
	header->pred = NULL;
	header->succ = trailer;
	trailer->pred = header;
	trailer->succ = NULL;
}

template <typename T>
void List<T>::copyNodes(ListNodePosi<T> p, Rank n)
{
	init();
	while (n--) {
		insertAsLast(p->data);
		p = p->succ;
	}
}

template <typename T>
Rank List<T>::clear()
{
	Rank ret = _size;
	while (!empty())
		remove(header->succ);
	return ret;
}


template <typename T> 
ListNodePosi<T> List<T>::insert( ListNodePosi<T> p, T const& e )
{
	_size++; 
	return p->insertAsSucc( e ); 
} // e当作p的后继插入

template <typename T> 
ListNodePosi<T> List<T>::insert( T const& e, ListNodePosi<T> p )
{
	_size++; 
	return p->insertAsPred( e ); 
} // e当作p的前驱插入


template <typename T>
T List<T>::remove(ListNodePosi<T> p)
{
	T val = p->data;

	ListNodePosi<T> p1 = p->pred;
	ListNodePosi<T> p2 = p->succ;
	p1->succ = p2;
	p2->pred = p1;

	delete p;
	_size--;

	return val;
}

// 无序查找, 在 p 的 n 个前驱中找
template <typename T>
ListNodePosi<T> List<T>::find(T const& e, Rank n,
ListNodePosi<T> p ) const
{
	while (0 < n--) {
		ListNodePosi<T> pre = p->pred;
		if (pre->data == e)
			return pre;
	}
	return NULL;
}

// 有序查找，[ p-n, p) 返回不大于 e 的最后一个节点
template <typename T>
ListNodePosi<T> List<T>::search( T const& e, Rank n, ListNodePosi<T> p ) const
{
	p = p->pred;
	while (n != 0 && e < p->data) {
		p=p->pred;
		n--;
	}
	return p;
}


//对列表中起始于位置p、宽度为n的区间做插入排序
template <typename T> 
void List<T>::insertionSort( ListNodePosi<T> p, Rank n )
{ 
   for ( Rank r = 0; r < n; r++ ) { //逐一为各节点
      insert( search( p->data, r, p ), p->data ); //查找适当的位置并插入
      p = p->succ; remove( p->pred ); //转向下一节点
   }
}


// 实现动态双链表
struct node {
	int data;
	struct node *next;
	struct node *pre;
};

struct node *head = (struct node*)malloc(sizeof(struct node));
head->next = head;
head->pre = head;

while (n-- > 0) {
	struct node *ptr = (struct node*)malloc(sizeof(struct node));
	struct node *pred = head->pred;
	struct node *next = head;
	ptr->next = next;
	ptr->pred = pred;
	pred->next = ptr;
	next->pred = ptr;
}


// 用数组实现双链表
struct node {
	int id;
	int nextid,predid;
};

struct node arr[MAX];

for (int i = 0; i < n; ++i)
{
	arr[i].id = i;
	arr[i].nextid = i+1;
	arr[i].predid = i-1;
}
arr[0].predid = n-1;
arr[n-1].nextid = 0;



#endif