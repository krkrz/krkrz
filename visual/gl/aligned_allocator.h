

#ifndef __ALIGNED_ALLOCATOR_H__
#define __ALIGNED_ALLOCATOR_H__

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__MINGW32__)
#include <malloc.h>		// _aligned_malloc and _aligned_free
#else
#include <stdlib.h>
#endif
#include <memory>		// std::allocator


// STL allocator
template< class T, int TAlign=16 >
struct aligned_allocator : public std::allocator<T>
{
	static const int ALIGN_SIZE = TAlign;
	template <class U> struct rebind    { typedef aligned_allocator<U,TAlign> other; };
	aligned_allocator() throw() {}
	aligned_allocator(const aligned_allocator&) throw () {}
	template <class U> aligned_allocator(const aligned_allocator<U, TAlign>&) throw() {}
	template <class U> aligned_allocator& operator=(const aligned_allocator<U, TAlign>&) throw()  {}
	// allocate
#if defined(_MSC_VER)
	pointer allocate(size_type c, const void* hint = 0) {
		return static_cast<pointer>( _mm_malloc( sizeof(T)*c, TAlign ) );
	}
#elif defined(__MINGW32__)
	T* allocate(std::size_t c) {
		return static_cast<T*>( _aligned_malloc( sizeof(T)*c, TAlign ) );
	}
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	T* allocate( std::size_t c ) {
		T* ret;
		posix_memalign(reinterpret_cast<void**>(&ret), TAlign, sizeof(T)*c);
		return ret;
	}
#else
	T* allocate( std::size_t c ) {
		return static_cast<T*>( aligned_alloc( TAlign, sizeof(T)*c ) );
	}
#endif
	// deallocate
#if defined(_MSC_VER)
	void deallocate(pointer p, size_type n) {
		_mm_free( p );
	}
#elif defined(__MINGW32__)
	void deallocate( T* p, std::size_t n ) {
		_aligned_free( p );
	}
#else
	void deallocate( T* p, std::size_t n ) {
		free( p );
	}
#endif
};


#endif // __ALIGNED_ALLOCATOR_H__


