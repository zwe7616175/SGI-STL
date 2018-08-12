#pragma once

// POD
struct TrueType
{};

struct FalseType
{};

template<class T>
struct TypeTraits
{
	typedef FalseType PODType;
};

//////偏特化 萃取出内置类型，用于destory()的时候提升效率
template<>
struct TypeTraits<int>
{
	typedef TrueType PODType;
};

template<>
struct TypeTraits<char>
{
	typedef TrueType PODType;
};


