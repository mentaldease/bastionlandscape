#if !defined __ATOMICQUEUE_PROCESSOR_H__
#define __ATOMICQUEUE_PROCESSOR_H__


// Note: MSVC may require /Oi compiler option

// CPU load/store reordering barriers and fast yield
// TODO: For other than _MSC_VER and __ICL
#if defined _M_IA64 // TODO: Probably no IA64 support needed
extern "C" __mf
#pragma intrinsic(__mf)
#define MemoryBarrier __mf
extern "C" void __yield(void);
#pragma intrinsic(__yield)
#define YieldProcessor __yield
#else
#if !defined __ICL || __ICC || __ECC
extern "C" void _mm_mfence(void);
#pragma intrinsic(_mm_mfence)
extern "C" void _mm_lfence(void);
#pragma intrinsic(_mm_lfence)
extern "C" void _mm_sfence(void);
#pragma intrinsic(_mm_sfence)
extern "C" void _mm_pause(void);
#pragma intrinsic(_mm_pause)
#endif
#ifdef YieldProcessor
#undef YieldProcessor
#endif // YieldProcessor
#define YieldProcessor _mm_pause
#if defined _M_AMD64 // TODO: Check if this is OK if load barrier needed
extern "C" void __faststorefence(void);
#pragma intrinsic(__faststorefence)
#define MemoryBarrier __faststorefence
#elif defined _M_IX86
/*__forceinline void MemoryBarrier(void)
{
long Barrier;
__asm xchg Barrier, eax
}*/
#define MemoryBarrier _mm_mfence
#else
#error "Unsupported environment"
#endif
#endif

// Compiler load/store reordering barriers
// TODO: Check if these are implied by the CPU barriers; __ECC should use explicit ...with acquire/release instructions instead
#if defined __ICL || __ICC || __ECC
#define _ReadWriteBarrier __memory_barrier
#define _ReadBarrier __memory_barrier
#define _WriteBarrier __memory_barrier
#elif defined _MSC_VER
extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)
extern "C" void _ReadBarrier(void);
#pragma intrinsic(_ReadBarrier)
extern "C" void _WriteBarrier(void);
#pragma intrinsic(_WriteBarrier)
#elif defined __GNUC__ // TODO: Other options for read- or write-only barriers?
#define _ReadWriteBarrier() __asm__ __volatile__("" ::: "memory")
#define _ReadBarrier() __asm__ __volatile__("" ::: "memory")
#define _WriteBarrier() __asm__ __volatile__("" ::: "memory")
#else
#error "Unsupported environment"
#endif

template<typename T>
__forceinline T Acquire(T volatile const &p)
{
	T t = p;
	_ReadWriteBarrier();
	return t;
}

template<typename S, typename T>
__forceinline void Release(volatile S &p, T const t)
{
	_ReadWriteBarrier();
	p = t;
}

// Interlocked intrinsics
#if 0
// TODO: For other than _MSC_VER and __ICL; note that __GNUC__ __sync_bool_compare_and_swap does the comparison but _Interlocked... doesn't
#if !defined __ICL || __ICC || __ECC
extern "C" long _InterlockedCompareExchange(volatile unsigned long *, unsigned long, unsigned long);
#pragma intrinsic(_InterlockedCompareExchange)
extern "C" long long _InterlockedCompareExchange64(volatile unsigned long long *, unsigned long long, unsigned long long);
#pragma intrinsic(_InterlockedCompareExchange64)
#endif
#else
#pragma intrinsic(_InterlockedCompareExchange)
extern "C" long long _InterlockedCompareExchange64(volatile unsigned long long *, unsigned long long, unsigned long long);
#pragma intrinsic(_InterlockedCompareExchange64)
#endif

// Aligned memory allocator and deallocator
#if !defined __ICL || __ICC || __ECC
#if defined _MSC_VER
#include <malloc.h>
#elif defined __GNUC__
#include <mm_malloc.h>
#else
#error "Unsupported environment"
#endif
#endif

template<bool> struct CompileError;
template<> struct CompileError<true>
{
};
#define STATIC_ASSERT(X) (CompileError<(X)>())

template<typename S, typename T>
union Pack
{
	inline Pack(void);
	inline Pack(Pack const &);
	inline Pack(unsigned long long const &);
	inline Pack(S const &, T const &);
	unsigned long long pack;
	struct // TODO: Check if the below order is only correct for little-endian architectures
	{
		S lower;
		T upper;
	};
};

template<typename S, typename T>
inline Pack<S, T>::Pack(void)
{
	STATIC_ASSERT(sizeof(S) == 4 && sizeof(T) == 4);
}

template<typename S, typename T>
inline Pack<S, T>::Pack(Pack const &other)
{
	STATIC_ASSERT(sizeof(S) == 4 && sizeof(T) == 4);
	pack = other.pack;
}

template<typename S, typename T>
inline Pack<S, T>::Pack(unsigned long long const &op)
{
	STATIC_ASSERT(sizeof(S) == 4 && sizeof(T) == 4);
	pack = op;
}

template<typename S, typename T>
inline Pack<S, T>::Pack(S const &nl, T const &nu)
{
	STATIC_ASSERT(sizeof(S) == 4 && sizeof(T) == 4);
	lower = nl;
	upper = nu;
}

#endif __ATOMICQUEUE_PROCESSOR_H__
