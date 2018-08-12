#include "alloc.h"
template<class T, class alloc>
class SimpleAlloc
{
public:
	//申请n个T类型的大小的空间
	static T* Allocate(size_t n)
	{
		return n == 0 ? 0 : (T*)alloc::Allocate(n*sizeof(T));
	}
	//申请1个T类型的大小的空间
	static T* Allocate()
	{
		return (T*)alloc::Allocate(sizeof(T));
	}
	//释放n个T类型的大小的空间
	static void Deallocate(T*p, size_t n)
	{
		if (n != 0)
			alloc::Deallocate(p, n*sizeof(T));
	}
	//释放1个T类型的大小的空间
	static void Deallocate(T*p)
	{
		alloc::Deallocate(p, sizeof(T));
	}
};
