/*
	算法证明：
	1. 不变性；
	2. 单调性；

*/

## 类型定义
```cpp
#include <cmath>


typedef unsigned int Rank;
#define DEFAULT_CAPACITY  (16)

template <typename T> class Vector { 
protected:
   Rank _size; 
   Rank _capacity;  
   T* _elem; //规模、容量、数据区

   void copyFrom ( T const* A, Rank lo, Rank hi ); 

   void expand(); //空间不足时扩容
   void shrink(); //装填因子过小时压缩


   void merge ( Rank lo, Rank mi, Rank hi ); //归并算法
   void mergeSort ( Rank lo, Rank hi ); //归并排序算法


public:
	// 构造函数
   Vector ( Rank c = DEFAULT_CAPACITY, Rank s = 0, T v = 0 );
   Vector ( Vector<T> const& V ); 
   Vector<T> & operator= ( Vector<T> const& ); 
   ~Vector() { delete [] _elem; }


   Rank size() const { return _size; }
   bool empty() const { return !_size; }
   T& operator[] ( Rank r ); 
   const T& operator[] ( Rank r ) const; 

   Rank insert ( Rank r, T const& e ); //插入元素
   Rank insert ( T const& e ) { return insert ( _size, e ); } // 插入末尾

   T remove ( Rank r ) {return remove(r, r+1);}
   Rank remove ( Rank lo, Rank hi ); // [lo, hi)

   Rank find ( T const& e ) const { return find ( e, 0, _size ); } //无序向量整体查找
   Rank find ( T const& e, Rank lo, Rank hi ) const; //无序向量区间查找

   Rank search ( T const& e ) const //有序向量整体查找
   { return ( 0 >= _size ) ? -1 : search ( e, 0, _size ); }
   Rank search ( T const& e, Rank lo, Rank hi ) const; //有序向量区间查找

//    void sort ( Rank lo, Rank hi ); //对[lo, hi)排序
//    void sort() { sort ( 0, _size ); } //整体排序
}; 
```


## 类初始化
```CPP
template <typename T>
Vector<T>::Vector(Rank capacity, Rank size, T val)
{
	_elem = new T[_capacity = capacity];
	for (_size = 0; _size < size;)
		_elem[_size++] = val;
}

template <typename T>
Vector<T>::Vector(Vector<T> const& V) 
{
	copyFrom ( V._elem, 0, V._size ); 
}

template <typename T>
Vector<T> & Vector<T>::operator= ( Vector<T> const& V)
{
	if (_elem)
		delete [] _elem;
	
	copy(V._elem, 0, V._size );
	return *this;
}

template <typename T>
void Vector<T>::copyFrom ( T const* A, Rank lo, Rank hi )
{
	_capacity = fmax(DEFAULT_CAPACITY, 2 * ( hi - lo ) );
	_elem = new T[_capacity];
	for ( _size = 0; lo < hi; _size++, lo++ )
		_elem[_size] = A[lo];
}

```

## 空间动态调整
```cpp
// 平均时间复杂度: O(1)

template <typename T>
void Vector<T>::expand()
{
	if (_size < _capacity) 
		return ;

	_capacity <<= 1;
	T *old_elem = _elem;
	_elem = new T[_capacity];
	for (Rank i = 0; i < _size; ++i)
		_elem[i] = old_elem[i];
	delete [] old_elem;
}

template <typename T>
void Vector<T>::shrink()
{
	if (_capacity < 2* DEFAULT_CAPACITY)
		return ;
	// 超过25%，不收缩
	if (_size > _capacity>>2)
		return ;

	_capacity >>= 1;
	T *old_elem = _elem;
	_elem = new T[_capacity];
	for (Rank i = 0; i < _size; ++i)
		_elem[i] = old_elem[i];
	delete [] old_elem;
}
```

## 插入和删除
```cpp
template <typename T>
T& Vector<T>::operator[] (Rank r)
{
	return _elem[r];
}

template <typename T>
const T& Vector<T>::operator[] (Rank r) const
{
	return _elem[r];
}


template <typename T>
Rank Vector<T>::insert(Rank r, T const &e) 
{
	expand();
	for (Rank i = _size; i > r; --i)
		_elem[i] = _elem[i-1];
	_elem[r] = e;
	_size++;
	return r;
}

// remove [lo, hi)
template <typename T>
Rank Vector<T>::remove(Rank lo, Rank hi) 
{
	if (lo == hi) return ;
	while (hi < _size) {
		_elem[lo] = _elem[hi];
		lo++;
		hi++;
	}
	_size = lo;
	shrink();
	return hi - lo;
}
```

## 查找
```cpp
 // 查找 [lo, hi), 失败时，返回 lo-1
template <typename T>
Rank Vector<T>::find(T const& e, Rank lo, Rank hi) const
{
	while (lo < hi--) {
		if (_elem[hi] == e)
			return hi;
	}
	return hi;
}


// 二分查找是把 pivot 定位中点；
// 如何进一步优化？观察到：向左每次要比较一次，向右却要比较2次，
// 更优算法1：pivot = 0.618
template <typename T>
Rank Vector<T>::search(T *A, T const& e, Rank lo, Rank hi)
{
	while (lo < hi) {
		Rank mid = (lo + hi) >> 1;
		if (e < A[mid]) hi = mid;
		else if (A[mid] < e) lo = mid+1;
		else                 return mid;
	}
	return -1;
}

// 更优算法2：改为2个分支，使向左和向右仅比较一次；
// 返回不大于 e 的元素的最大秩
template <typename T>
Rank Vector<T>::search(T *A, T const& e, Rank lo, Rank hi)
{
	while (lo < hi) {
		Rank mid = (lo + hi) >> 1;
		if (e < A[mid]) hi = mid; // [lo, mid)
		else lo = mid + 1;  
		// (mid, hi), A[mid] <= e, 
		// (mid,hi)存在等于e的元素，当然可以放弃A[mid]
		// (mid, hi) 每个元素大于 e， lo 和 hi 会相等，最后还是得到mid
	} // 出口 lo == hi
	return (lo-1);
}
```

## 合并排序
```cpp
template <typename T>
void Vector<T>::merge(Rank lo, Rank mi, Rank hi)
{
	// A 重新排列后的 vector
	// B 复制出 [lo, mi)
	// C [mi,hi)
    Rank i = 0; T* A = _elem + lo; 
    Rank j = 0, lb = mi - lo; T* B = new T[lb]; 
    for ( i = 0; i < lb; i++ ) B[i] = A[i]; //复制出 [lo, mi)

	Rank k = 0, lc = hi - mi; T* C = _elem + mi; 
	
	i = 0;
    while ( ( j < lb ) && ( k < lc ) ) //反复地比较B、C的首元素
       A[i++] = ( B[j] <= C[k] ) ? B[j++] : C[k++]; 
    while ( j < lb ) //若C先耗尽，则
       A[i++] = B[j++]; //将B残余的后缀归入A中——若B先耗尽呢？
    delete[] B; 
}


template <typename T>
void Vector<T>::mergeSort(Rank lo, Rank hi)
{
	if (hi <= lo) return ;
	Rank mid = (lo + hi) >> 1;
	mergeSort(lo, mid);
	mergeSort(mid, hi);
	merge(lo, mid, hi);
}
```