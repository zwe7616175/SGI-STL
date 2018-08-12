#include "SimpleAlloc.h"
#include "Construct.h"

template<class T>
struct ListNode
{
	ListNode<T>* _pNext;
	ListNode<T>* _pPre;
	T _data;
};


template<class T, class Ref, class Ptr>
struct ListIterator
{
	typedef ListIterator<T, Ref, Ptr> Self;
	typedef ListNode<T> Node;
	typedef BidirectionalIteratorTag IteratorCategory;
	typedef T ValueType;
	typedef Ref Reference;
	typedef Ptr Pointer;
	typedef int DifferenceType;

public:
	ListIterator(Node* p = NULL)
		: _pNode(p)
	{}

	ListIterator(const Self& it)
		: _pNode(it._pNode)
	{}

	Ref operator*()
	{
		return _pNode->_data;
	}

	Ptr operator->()
	{
		return &(_pNode->_data);
	}

	Self& operator++()
	{
		_pNode = _pNode->_pNext;
		return *this;
	}

	Self operator++(int)
	{
		Self temp(*this);
		++*this;
		return temp;
	}

	Self& operator--()
	{
		_pNode = _pNode->_pPre;
		return *this;
	}

	Self operator--(int)
	{
		Self temp(*this);
		--*this;
		return temp;
	}

	bool operator==(const Self& it)
	{
		return _pNode == it._pNode;
	}

	bool operator!=(const Self& it)
	{
		return _pNode != it._pNode;
	}

private:
	Node* _pNode;
};

template<class T, class _Alloc_ = default_Alloc<0> >
class List
{
	typedef ListNode<T> Node;
	typedef SimpleAlloc<Node, _Alloc_> ListAlloc;

public:
	typedef ListIterator<T, T&, T*> Iterator;
	typedef ReverseIterator<Iterator> ReverseIterator;
public:
	Iterator Begin()
	{
		return Iterator(_pHead->_pNext);
	}

	Iterator End()
	{
		return Iterator(_pHead);
	}

	ReverseIterator RBegin()
	{
		return ReverseIterator(End());
	}

	ReverseIterator REnd()
	{
		return ReverseIterator(Begin());
	}

	List()
	{
		_pHead = CreateListNode();
		_pHead->_pNext = _pHead;
		_pHead->_pPre = _pHead;
	}

	void PushBack(const T& data)
	{
		Node* pNewNode = CreateListNode(data);

		pNewNode->_pNext = _pHead;
		pNewNode->_pPre = _pHead->_pPre;
		_pHead->_pPre->_pNext = pNewNode;
		_pHead->_pPre = pNewNode;
	}

	void PopBack()
	{
		if (_pHead->_pNext == _pHead)
			return;

		Node* pDel = _pHead->_pPre;
		pDel->_pPre->_pNext = _pHead;
		_pHead->_pPre = pDel->_pPre;

		Destroy(pDel);
	}

	void PushFront(const T& data)
	{
		Node* pNewNode = CreateListNode(data);

		pNewNode->_pNext = _pHead->_pNext;
		pNewNode->_pPre = _pHead;

		_pHead->_pNext = pNewNode;
		pNewNode->_pNext->_pPre = pNewNode;
	}

	void PopFront()
	{
		if (_pHead->_pNext == _pHead)
			return;

		Node* pDel = _pHead->_pNext;
		pDel->_pNext->_pPre = _pHead;
		_pHead->_pNext = pDel->_pNext;
		Destroy(pDel);
	}

	Iterator Insert(Iterator pos, const T& data)
	{
		Node* pNewNode = CreateListNode(data);
		pNewNode->_pNext = pos._pNode;
		pNewNode->_pPre = pos._pNode->_pPre;
		pos._pNode->_pPre = pNewNode;
		pNewNode->_pPre->_pNext = pNewNode;

		return Iterator(pNewNode);
	}

	Iterator Erase(Iterator pos, const T& data)
	{
		Node* pDel = pos._pNode;
		Node* pRet = pDel->_pNext;
		pDel->_pNext->_pPre = pDel->_pPre;;
		pDel->_pPre->_pNext = pDel->_pNext;

		Destroy(pDel);
		return Iterator(pRet);
	}


	void Clear()
	{
		Node* pCur = _pHead->_pNext;
		while (pCur != _pHead)
		{
			_pHead->_pNext = pCur->_pNext;
			Destroy(&pCur->_data);
			pCur = _pHead->_pNext;
		}

		_pHead->_pNext = _pHead;
		_pHead->_pPre = _pHead;
	}

	size_t Size()const
	{
		size_t count = 0;
		Iterator it = Begin();
		while (it != End())
		{
			++count;
			++it;
		}
		return count;
	}

	bool Empty()const
	{
		return _pHead->_pNext == _pHead;
	}

	~List()
	{
		Clear();
		Destroy(&_pHead->_data);
		_pHead = NULL;
	}

private:
	Node* GetNode()
	{
		return ListAlloc::Allocate();
	}

	void PutNode(Node* p)
	{
		return ListAlloc::DeAllocate(p);
	}

	Node* CreateListNode(const T& value = T())
	{
		Node* newNode = GetNode();
		Construct(&newNode->_data, value);
		return newNode;
	}

	void DestroyListNode(Node* p)
	{
		Destroy(&p->_data);
		PutNode(p);
	}
private:
	Node* _pHead;
};

#include <iostream>
using namespace std;

#include<string>

void TestListString()
{
	List<string> l;
	l.PushBack("1111");
	l.PushBack("2222");
	l.PushBack("3333");
	l.PushBack("4444");

	List<string>::Iterator it = l.Begin();
	while (it != l.End())
	{
		cout << *it << " ";
		++it;
	}
	cout << endl;

	cout << Difference(l.Begin(), l.End()) << endl;

	List<string>::ReverseIterator rIt = l.RBegin();
	while (rIt != l.REnd())
	{
		cout << *rIt << " ";
		++rIt;
	}
	cout << endl;
}

void TestListInt()
{
	List<int> l;
	l.PushBack(1111);
	l.PushBack(2222);
	l.PushBack(3333);
	l.PushBack(4444);

	List<int>::Iterator it = l.Begin();
	while (it != l.End())
	{
		cout << *it << endl;
		++it;
	}
	cout << endl;
}
