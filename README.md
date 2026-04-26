This is a very simple wrapper for `malloc`-like APIs
which I used to estimate heap autoinit overheads in
my [Rust performance analysis](https://github.com/yugr/rust-slides/).

To use, simply compile via
```
$ gcc -O2 -fPIC -shared zeralloc.c -ldl -o libzeralloc.so
```
and run
```
$ LD_PRELOAD=libzeralloc.so ./prog
```

Limitations:
  - Linux only
  - Huge RAM and performance overheads (a good solution should do `mmap`
    instead of resetting bytes).
  - Won't work for programs that link against static allocator library
  - A better (but less ergonomic) approach would be to use compiler plugin
    (to allow compiler to remove redundant inits)
