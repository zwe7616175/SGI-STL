#pragma once
/////用来区分不同类型的迭代器，在求Difference时候根据不同类型的迭代器调用相应的函数
struct InputIteratorTag
{};

struct OutputIteratorTag
{};

struct ForwardIteratorTag : public InputIteratorTag
{};

struct BidirectionalIteratorTag : public ForwardIteratorTag
{};

struct RandomAccessIteratorTag : public BidirectionalIteratorTag
{};


template <class T, class Distance>
struct BidirectionalIterator
{
	typedef BidirectionalIteratorTag IteratorCategory;
	typedef T                          ValueType;
	typedef Distance                   DifferenceType;
	typedef T*                         Pointer;
	typedef T&                         Reference;
};

template <class T, class Distance>
struct RandomAccessIterator
{
	typedef RandomAccessIteratorTag IteratorCategory;
	typedef T                          ValueType;
	typedef Distance                   DifferenceType;
	typedef T*                         Pointer;
	typedef T&                         Reference;
};

// 迭代器：自定义的迭代器
template<class Iterator>
struct IteratorTraits
{
	typename typedef Iterator::IteratorCategory IteratorCategory;
	typename typedef Iterator::ValueType ValueType;
	typename typedef Iterator::Reference Reference;
	typename typedef Iterator::Pointer Pointer;
	typename typedef Iterator::DifferenceType DifferenceType;
};

// 偏特化
/////////由于内置类型的迭代器是原生指针，所以要特殊处理
template<class T>
struct IteratorTraits<T*>
{
	typedef RandomAccessIteratorTag IteratorCategory;
	typedef T ValueType;
	typedef T& Reference;
	typedef T* Pointer;
	typedef int  DifferenceType;
};

// STL: 追求效率
// 双向 O(N)
template<class Iterator>
typename IteratorTraits<Iterator>::DifferenceType Difference(Iterator first, Iterator last, BidirectionalIteratorTag)
{
	typename IteratorTraits<Iterator>::DifferenceType diff = 0;
	while (first != last)
	{
		++diff;
		++first;
	}
	return diff;
}

// O(1)
template<class Iterator>
typename IteratorTraits<Iterator>::DifferenceType Difference(Iterator first, Iterator last, RandomAccessIteratorTag)
{
	return last - first;
}

// STL: 效率
template<class Iterator>
typename IteratorTraits<Iterator>::DifferenceType Difference(Iterator first, Iterator last)
{
	return Difference(first, last, IteratorTraits<Iterator>::IteratorCategory());
}


// ReverseIterator: 正向迭代器的适配器
template<class Iterator>
class ReverseIterator
{
	typedef ReverseIterator<Iterator> Self;

public:

	typename typedef IteratorTraits<Iterator>::IteratorCategory IteratorCategory;
	typename typedef IteratorTraits<Iterator>::ValueType ValueType;
	typename typedef IteratorTraits<Iterator>::Reference Reference;
	typename typedef IteratorTraits<Iterator>::Pointer Pointer;
	typename typedef IteratorTraits<Iterator>::DifferenceType DifferenceType;

public:
	ReverseIterator()
	{}

	ReverseIterator(Iterator it)
		: _it(it)
	{}

	ReverseIterator(Self& s)
		: _it(s._it)
	{}

	Reference operator*()
	{
		Iterator temp(_it);
		--temp;
		return *temp;
	}

	Pointer operator->()
	{
		return &(operator*());
	}

	Self& operator++()
	{
		--_it;
		return *this;
	}

	Self operator++(int)
	{
		Iterator temp(_it);
		_it--;
		return temp;
	}

	Self& operator--()
	{
		++_it;
		return *this;
	}

	Self operator--(int)
	{
		Iterator temp(_it);
		_it++;
		return temp;
	}

	bool operator!=(const Self& s)
	{
		return _it != s._it;
	}

	bool operator==(const Self& s)
	{
		return _it == s._it;
	}

private:
	Iterator _it;
};
