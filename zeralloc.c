/*
 * Copyright 2026 Yury Gribov
 *
 * The MIT License (MIT)
 * 
 * Use of this source code is governed by MIT license that can be
 * found in the LICENSE.txt file.
 */

#define _GNU_SOURCE

#include <stdint.h>
#include <dlfcn.h>

// To my knowledge the code below
//   - is thread-safe
//   - works for C++ new operators (they use malloc under the hood)

size_t malloc_usable_size (void *ptr);
void *memset(void *s, int c, size_t n);

// Malloc and realloc are called inside dlsym
// so need to be careful
// (this will break sanitizers so better add a ctor to initialize pointers and
// use a temp buffer for dlsym allocations)
void *__libc_malloc(size_t size);
void *__libc_realloc(void *ptr, size_t size);

#define REAL(name)                   \
  static __typeof__(name) *real = 0; \
  if (!real)                         \
    real = dlsym(RTLD_NEXT, #name);

void *malloc(size_t size) {
  // Call calloc here and in other places for more efficient allocation ?
  // But this may introduce infinite recursion...
  void *res = __libc_malloc(size);
  if (res)
    memset(res, 0, size);
  return res;
}

void *realloc(void *ptr, size_t size) {
  // TODO: malloc_size on OSX, tc_malloc_usable_size in tcmalloc
  size_t old_size = ptr ? malloc_usable_size(ptr) : 0;
  char *res = (char *)__libc_realloc(ptr, size);
  if (res && size > old_size)
    memset(res + old_size, 0, size - old_size);
  return res;
}

// Calloc already zeroes memory

void *reallocarray(void *ptr, size_t nmemb, size_t size) {
  REAL(reallocarray);
  // TODO: malloc_size on OSX, tc_malloc_usable_size in tcmalloc
  size_t old_size = ptr ? malloc_usable_size(ptr) : 0;
  char *res = (char *)real(ptr, nmemb, size);
  if (!res)
    return res;
  // reallocarray guarantees that there is no overflow at this point
  size_t len = nmemb * size;
  if (len > old_size)
    memset(res + old_size, 0, len - old_size);
  return res;
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
  REAL(posix_memalign);
  int res = real(memptr, alignment, size);
  if (res == 0 && *memptr)
    memset(*memptr, 0, size);
  return res;
}

void *aligned_alloc(size_t alignment, size_t size) {
  REAL(aligned_alloc);
  void *res = real(alignment, size);
  if (res)
    memset(res, 0, size);
  return res;
}

void *valloc(size_t size) {
  REAL(valloc);
  void *res = real(size);
  if (res)
    memset(res, 0, size);
  return res;
}

void *memalign(size_t alignment, size_t size) {
  REAL(memalign);
  void *res = real(alignment, size);
  if (res)
    memset(res, 0, size);
  return res;
}

void *pvalloc(size_t size) {
  REAL(pvalloc);
  void *res = real(size);
  // TODO: round size to page size
  if (res)
    memset(res, 0, size);
  return res;
}
