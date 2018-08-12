#pragma once
#include <new>
#include <iostream>
using namespace std;

//一级空间配置器
#define THROW_BAD_ALLOC cerr<<"out of memory"<<endl;exit(1)

template <int inst>
class Alloc_malloc
{
public:
	//Allocate用于申请空间
	static void* Allocate(size_t n)
	{
		void* result = malloc(n);
		//申请失败，使用oom_malloc()重新尝试申请
		if (result == NULL)
			return Oom_malloc(n);
		return NULL;
	}

	//Deallocate用于释放空间
	static void Deallocate(void* p, size_t /* size*/)
	{
		free(p);
	}

	//Reallocate用于根据需要调整已经存在的空间的大小
	static void* Reallocate(void* p, size_t size)
	{
		void* result = realloc(p, size);
		//申请失败，使用oom_realloc()尝试申请
		if (result == NULL)
			return Oom_realloc(p, size);
		return NULL;
	}

	//Set_malloc_handler函数是用于设置用户提供的释放空间的函数指针
	static void(*Set_malloc_handler(void(*f)())) ()
	{
		void(*old)() = _Malloc_alloc_oom_handler;
		_Malloc_alloc_oom_handler = f;
		return (old);
	}

private:
	//通过用户提供的释放空间（释放自己已经不用了的空间）的函数不断的释放空间并检测
	//直到释放出的空间足够分配给申请的空间
	//如果用户没有提供释放空间的函数，则抛异常
	static void* Oom_malloc(size_t size)
	{
		void* result;
		void(*My_malloc_handler)();
		for (;;)
		{
			My_malloc_handler = _Malloc_alloc_oom_handler;
			if (0 == My_malloc_handler)//用户没有提供释放空间的函数
				THROW_BAD_ALLOC;
			(*My_malloc_handler)();
			result = malloc(size);
			if (result)
				return result;
		}
	}

	static void* Oom_realloc(void* p, size_t size)
	{
		void* result;
		void(*My_malloc_handler) ();
		for (;;)
		{
			My_malloc_handler = _Malloc_alloc_oom_handler;
			if (0 == my_malloc_handler)
				THROW_BAD_ALLOC;
			(*My_malloc_handler)();
			result = realloc(p, size);
			if (result)
				return result;
		}
	}

private:
	static void(*_Malloc_alloc_oom_handler)();
};

//类外初始化静态成员变量 _malloc_alloc_oom_handler
template <int inst>
void(*Alloc_malloc<inst>::_Malloc_alloc_oom_handler)() = 0;

//二级空间配置器
enum{ ALINE = 8 };//区块的划分边界，如果为4空间利用率不高，分配时还需要一部分负载去管理
enum{ MAX_BYTES = 128 };//小内存分配的界限
enum{ NFREELISTS = MAX_BYTES / ALINE };//free_list的个数

template <int inst>
class default_Alloc
{
public:
	static void* Allocate(size_t size)//开辟空间
	{
		obj* volatile result;
		//大空间，使用一级空间配置器分配内存
		printf("申请%d字节的空间\n", size);
		if (size > MAX_BYTES)
		{
			printf("申请空间大于128字节，使用一级空间配置器\n");
			return Alloc_malloc<inst>::Allocate(size);
		}

		//小空间，使用二级空间配置器分配内存
		//先去对应的自由链表中找
		int index = Free_list_index(size);
		result = _free_list[index];
		if (NULL == result)//自由链表中没有空间，从内存池里申请空间
		{
			printf("第%d号自由链表中没有空间，向内存池中申请空间\n", index);
			result = (obj*)Refill(size);
			return result;
		}
		_free_list[index] = result->free_list_link;
		printf("在第%d号自由链表中获取到%d个字节的空间\n", index, size);
		return result;
	}

	static void Deallocate(void* p, size_t n)//释放空间
	{
		if (n > 128)
		{
			printf("释放%d个字节的空间，归还给系统\n", n);
			Alloc_malloc<inst>::Deallocate(p, n);
			return;
		}
		obj* pCur = (obj*)p;
		int index = Free_list_index(n);
		pCur->free_list_link = _free_list[index];
		_free_list[index] = pCur;
		printf("释放%d个字节的空间，挂接到%d号自由链表中\n\n", n, index);
	}

private:
	static size_t Round_up(size_t bytes)//用于上调至ALINE的整数倍
	{
		return (((bytes)+ALINE - 1)&~(ALINE - 1));
	}

	static size_t Free_list_index(size_t bytes)//用于计算在free_lists中的下标位置
	{
		return (bytes - 1) / ALINE;
		//return (bytes+(ALINE-1))/ALINE-1;  //SGI中的处理方式
	}
	static void* Refill(size_t bytes) //向内存池中申请空间
	{
		int nobjs = 20;
		//调用chunk_alloc函数，尝试获取nobjs个内存块
		char* chunk = Chunk_alloc(bytes, nobjs);
		obj* result;
		if (1 == nobjs)//只申请到一块空间的大小，将该块空间返回给用户使用
		{
			printf("从内存池里成功申请到%d个%d字节大小的内存块\n", nobjs, bytes);
			return chunk;
		}

		//否则代表有多块，将其中一块返回给用，其他的挂接到自由链表中
		printf("从内存池里成功申请到%d个%d字节大小的内存块\n", nobjs, bytes);
		result = (obj*)chunk;
		int n = Free_list_index(bytes);
		obj* next = (obj*)(chunk + bytes);
		while (1)
		{
			next->free_list_link = _free_list[n];
			_free_list[n];
			next = (obj*)((char*)next + bytes);
			if ((char*)next == chunk + nobjs*bytes)
			{
				printf("成功将剩下的%d个%d字节大小的内存块挂接到%d号自由链表中\n", nobjs - 1, bytes, n);
				break;
			}

		}
		return result;
	}


private:
	static char* Chunk_alloc(size_t size, int& nobjs)
	{
		char* result;
		size_t total_bytes = size*nobjs;
		size_t bytes_left = _end - _start;
		if (bytes_left >= total_bytes)//内存池里还有足够的空间
		{
			printf("向内存池申请%d个%d字节大小的内存块\n", nobjs, size);
			result = _start;
			_start += total_bytes;
			return result;
		}
		else if (bytes_left >= size)//内存池里边还能提供至少一块内存块
		{
			nobjs = bytes_left / size;
			result = _start;
			_start += nobjs*size;
			printf("向内存池申请%d个%d字节大小的内存块\n", nobjs, size);
			return result;
		}
		else
		{
			//内存池里边连一个内存块空间的大小都没有，朝系统里边申请空间填充内存池
			if (bytes_left > 0)//如果内存池里还剩有空间
			{
				int n = Free_list_index(bytes_left);
				((obj*)_start)->free_list_link = _free_list[n];
				_free_list[n] = (obj*)_start;
			}
			size_t bytetoget = 2 * total_bytes + Round_up(_heap_size >> 4);
			_start = (char*)malloc(bytetoget);
			printf("内存池空间不足%d字节，向系统申请%d字节大小的内存块\n", size, bytetoget);
			if (NULL == _start)//系统里边没有足够的空间
			{
				printf("系统空间不足，不能分配%d字节大小的内存块\n", bytetoget);
				//从当前自由链表开始遍历管理自由链表的数组
				for (size_t i = size; i <= MAX_BYTES; i++)
				{
					int n = Free_list_index(i);
					if (_free_list[n])//该自由链表里管理的有空间
					{
						_start = (char*)_free_list[n];
						_end = _start + i;
						printf("在第%d号自由链表中获取到%d个字节的空间放入内存池中\n", n, size);
						//递归调用自己，方便调整nobjs
						return (Chunk_alloc(size, nobjs));
					}
				}
				//已经是山穷水尽了，尝试调用一级空间配置器来申请
				printf("malloc已经失败，而且自由链表上也没有比他大的小块内存了，使用一级空间配置器来再次尝试申请空间\n");
				_end = 0;
				_start = (char*)Alloc_malloc<inst>::Allocate(size);
			}
			_end = _start + bytetoget;
			_heap_size += bytetoget;
			//递归调用自己，方便调整nobjs
			return (Chunk_alloc(size, nobjs));
		}
	}

private:
	union obj
	{
		union obj* free_list_link;
		char client_data[1];
	};

private:
	static char* _start;
	static char* _end;
	static size_t _heap_size;

	static obj* volatile _free_list[NFREELISTS];
};
//类外初始化类的静态成员变量
template <int inst>
char* default_Alloc<inst> ::_start = 0;

template <int inst>
char* default_Alloc<inst> ::_end = 0;

template <int inst>
size_t default_Alloc<inst> ::_heap_size = 0;

template <int inst>
typename default_Alloc<inst>::obj* volatile default_Alloc<inst>::_free_list[NFREELISTS] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
