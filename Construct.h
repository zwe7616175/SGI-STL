#pragma once
#include <new>
#include "TypeTraits.h"
#include "IteratorTraits.h"

template<class T1, class T2>
void Construct(T1* p, const T2& value)
{
	cout << "调用" << typeid(T1).name() << "构造函数" << endl;
	new(p)T1(value);
}

template<class T>
void Destroy(T* p)
{
	cout << "调用" << typeid(T).name() << "析构函数" << endl;
	p->~T();
}



// O(N)
// List---->[Begin(), End() )
template<class Iterator>
void _Destroy(Iterator first, Iterator last, FalseType)
{
	while (first != last)
		Destroy(&(*first++));
}


// O(1)
//如果存储的是内置类型，就不用调析构函数
template<class Iterator>
void _Destroy(Iterator first, Iterator last, TrueType)
{}


template<class Iterator>
void Destroy(Iterator first, Iterator last)
{
	_Destroy(first, last, TypeTraits<IteratorTraits<Iterator>::ValueType>::PODType());
}


