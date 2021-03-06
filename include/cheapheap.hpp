#ifndef CHEAPHEAP_H
#define CHEAPHEAP_H

#include "bufferbump.hpp"
#include "stack.hpp"
#include "classwarfare.hpp"
#include "tprintf.h"

template <unsigned long Size = 1024UL * 1048576UL,
	  unsigned long Multiple = 8,
	  unsigned long MinSize = 16>
class CheapHeap {
public:

  enum { NumClasses = ClassWarfare<Multiple>::getSizeClass(1024*1048576) };
	  
  enum { Alignment = 16 }; // Multiple };
  
  __attribute__((always_inline)) inline void * malloc(size_t sz) {
#if 1
    if (unlikely(sz < MinSize)) {
      sz = MinSize;
    }
#endif
    size_t rounded;
    int sizeClass;
    void * ptr;
    ClassWarfare<Multiple>::getSizeAndClass(sz, rounded, sizeClass);
    //    if (sz > 1048576) {
    //      tprintf::tprintf("CheapHeap::malloc(@) - @\n", sz, rounded);
    //    }
    if (likely(_arr[sizeClass].pop(ptr))) {
      // auto testSz = _buf.getSize(ptr);
      //	tprintf::tprintf("CheapHeap::malloc(@) = @\n", rounded, ptr);
      return ptr;
    } else {
      ptr = slowPathMalloc(rounded);
      //    tprintf::tprintf("CheapHeap::slowPathMalloc(@) = @\n", rounded, ptr);
      return ptr;
    }
  }
  
  __attribute__((always_inline)) inline void free(void * ptr) {
#if 1
    if (unlikely(ptr == nullptr)) {
      return;
    }
#endif
    //    tprintf::tprintf("CheapHeap::free:");
    auto sz = _buf.getSize(ptr);
    //    tprintf::tprintf("CheapHeap::free(@), sz=@\n", ptr, sz);
    if (unlikely(sz == 0)) { // check for out of bounds.
      return;
    }
    size_t rounded;
    int sizeClass;
    ClassWarfare<Multiple>::getSizeAndClass(sz, rounded, sizeClass);
    _arr[sizeClass].push(ptr);
  }

  size_t freedMemory() {
    size_t totalFreed = 0;
    for (auto i = 0; i < NumClasses; i++) {
      size_t sz;
      ClassWarfare<Multiple>::getSizeFromClass(i, sz);
      totalFreed += _arr[i].size() * sz;
    }
    return totalFreed;
  }
  
  __attribute__((always_inline)) inline size_t constexpr getSize(void * ptr) {
    //        tprintf::tprintf("CheapHeap::getSize(@):", ptr);
    auto sz = _buf.getSize(ptr);
    //        tprintf::tprintf("CheapHeap::getSize(@) = @\n", ptr, sz);
    return sz;
  }
  
private:

  __attribute__((noinline)) void * slowPathMalloc(size_t rounded) {
    return _buf.malloc(rounded);
  }
  
  Stack<void *> _arr[NumClasses];
  BufferBump<NumClasses, Size, Multiple> _buf;
};

#endif
