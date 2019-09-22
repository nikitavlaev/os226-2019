#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <malloc.h>
#include <string.h>

#define CONCAT(...) __VA_ARGS__

#if 1
#define FNX(_ret, _name, _decl, _call, _str_init, ...)      \
	static _ret (*_name##_p)(_decl);                        \
	void _name##_init(void) __attribute__((constructor));   \
	void _name##_init(void)                                 \
	{                                                       \
		_name##_p = dlsym(RTLD_NEXT, #_name);               \
		_str_init;                                          \
		fprintf(stderr, "init " #_name " %p\n", _name##_p); \
	}                                                       \
	_ret _name(_decl)                                       \
	{                                                       \
		__VA_ARGS__                                         \
	}

#define CONST_USED_MEMORY 8000 //size of allocated memory with any conditions
static void *pool;
static long long int pool_size = 0;
static long long int max_pool_size = CONST_USED_MEMORY;

FNX(void *, calloc,
	CONCAT(size_t nmemb, size_t size),
	CONCAT(nmemb, size),
	{},
	{
		fprintf(stderr, "%s nmemb %lu size %lu\n", __func__, nmemb, size);
		void *ptr = malloc(nmemb * size);
		memset(ptr, 0, size);
		return ptr;
	})

FNX(void *, realloc,
	CONCAT(void *ptr, size_t size),
	CONCAT(ptr, size),
	{},
	{
		long long int offset = (ptr - (void *)pool);
		fprintf(stderr, "%s ptr %lld size %lu\n", __func__, offset, size);
		if (offset < (long long int)sizeof(int))
		{
			fprintf(stderr, "incorrect realloc pointer");
			return NULL;
		}
		long long int old_size = *((int *)(ptr - sizeof(int)));
		fprintf(stderr, "%s ptr %p size %lld\n", __func__, ptr, old_size);
		void *dest = malloc(size);
		memcpy(dest, ptr, old_size);
		free(ptr);
		return dest;
	})

FNX(void *, malloc,
	size_t size,
	size,
	{
		pool = (void *)calloc_p(CONST_USED_MEMORY, sizeof(char));
	},
	{
		int i = 0;
		int neg_size = -size;
		if (pool_size < 8000) //common part of the program with "no" frees
		{
			if (pool_size + size > max_pool_size)
			{
				max_pool_size *= 2;
				pool = (void *)realloc_p(pool, max_pool_size);
				fprintf(stderr, "resized\n");
			}

			*((int *)(pool + pool_size)) = neg_size;
			*((int *)(pool + pool_size + sizeof(int) + size)) = neg_size;
			pool_size += size + 2 * sizeof(int);
		}
		else
		{

			while (i < pool_size)
			{
				if (*(int *)(pool + i) < 0 || abs(*(int *)(pool + i)) < size)
				{

					i += abs(*(int *)(pool + i)) + 2 * sizeof(int);
				}
				else
				{
					*((int *)(pool + i)) = neg_size;
					*((int *)(pool + i + sizeof(int) + size)) = neg_size;
					fprintf(stderr, "%s size %lu, found_free = %d\n", __func__, size, i);
					return pool + i + sizeof(int);
				}
			}
			if (i >= pool_size)
			{
				if (pool_size + size > max_pool_size)
				{
					max_pool_size *= 2;
					pool = (void *)realloc_p(pool, max_pool_size);
					fprintf(stderr, "resized\n");
				}

				*((int *)(pool + pool_size)) = neg_size;
				*((int *)(pool + pool_size + sizeof(int) + size)) = neg_size;
				pool_size += size + 2 * sizeof(int);
			}
		}
		fprintf(stderr, "%s size %lu, new_free = %lld, placed == %d\n",
				__func__, size, pool_size, *(int *)(pool + pool_size - 2 * sizeof(int) - size));
		return pool + pool_size - size - sizeof(int);
	})

FNX(void, free,
	void *ptr,
	ptr,
	{},
	{
		long long int offset = (ptr - (void *)pool);
		if (offset < 0)
		{
			fprintf(stderr, "incorrect free\n");
			return;
		}
		int size = -(*(int *)(ptr - sizeof(int)));
		int left_offset = 0;

		if (offset + size + sizeof(int) < pool_size)
		{
			int right_size = *((int *)(ptr + size + sizeof(int)));
			if (right_size >= 0)
			{
				size += right_size + sizeof(int);
			}
		}
		else
		{
			pool_size -= 2 * sizeof(int) + size;
		}

		if (offset > 2 * sizeof(int))
		{
			int left_size = *((int *)(ptr - 2 * sizeof(int)));
			if (left_size >= 0)
			{
				size += left_size + sizeof(int);
				left_offset += 2 * sizeof(int) + left_size;
			}
		}

		*((int *)(ptr - left_offset - sizeof(int))) = size;
		*((int *)(ptr - left_offset + size)) = size;

		fprintf(stderr, "%s ptr %ld size %d actual size %d\n",
				__func__, ptr - (void *)pool, *((int *)(ptr - sizeof(int))), size);
	})

#else

#define MALLOCFN_X(x)                                                         \
	x(void *, malloc, 1, size_t, size, "%lu")                                 \
		x(void, free, 1, void *, ptr, "%p")                                   \
			x(void *, calloc, 2, size_t, nmemb, "%lu", size_t, size, "%lu")   \
				x(void *, realloc, 2, void *, ptr, "%p", size_t, size, "%lu") \
					x(void *, reallocarray, 3, void *, ptr, "%p", size_t, nmemb, "%lu", size_t, size, "%lu")

#define DEFINE_(ret, name, dec) \
	ret (*name##_p)(dec);
#define DEFINE1(ret, name, type1, name1, fmt1) \
	DEFINE_(ret, name, CONCAT(type1 name1))
#define DEFINE2(ret, name, type1, name1, fmt1, type2, name2, fmt2) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2))
#define DEFINE3(ret, name, type1, name1, fmt1, type2, name2, fmt2, type3, name3, fmt3) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2, type3 name3))
#define DEFINE(ret, name, n, ...) \
	DEFINE##n(ret, name, ##__VA_ARGS__)
MALLOCFN_X(DEFINE)
#undef DEFINE_
#undef DEFINE1
#undef DEFINE2
#undef DEFINE3
#undef DEFINE

#define DEFINE_(ret, name, dec, call, fmt)               \
	ret name(dec)                                        \
	{                                                    \
		fprintf(stderr, "%s " fmt "\n", __func__, call); \
		return (ret)0; /* FIXME implement me */          \
		return name##_p(call);                           \
	}
#define DEFINE1(ret, name, type1, name1, fmt1) \
	DEFINE_(ret, name, CONCAT(type1 name1), CONCAT(name1), #name1 " " fmt1)
#define DEFINE2(ret, name, type1, name1, fmt1, type2, name2, fmt2) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2), CONCAT(name1, name2), #name1 " " fmt1 " " #name2 " " fmt2)
#define DEFINE3(ret, name, type1, name1, fmt1, type2, name2, fmt2, type3, name3, fmt3) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2, type3 name3), CONCAT(name1, name2, name3), #name1 " " fmt1 #name2 " " fmt2 " " #name3 " " fmt3)
#define DEFINE(ret, name, n, ...) \
	DEFINE##n(ret, name, ##__VA_ARGS__)
MALLOCFN_X(DEFINE)
#undef DEFINE_
#undef DEFINE1
#undef DEFINE2
#undef DEFINE3
#undef DEFINE

void init(void) __attribute__((constructor));
void init(void)
{
#define INIT(ret, name, n, ...) \
	name##_p = dlsym(RTLD_NEXT, #name);
	MALLOCFN_X(INIT)
#undef INIT
}

#endif
