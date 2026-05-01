typedef unsigned long size_t;

int abs(int i) { return i < 0 ? -i : i; }

void bcopy(const void *src, void *dest, size_t n) {
  char *d = (char*)dest;
  const char *s = (const char*)src;
  while (n--) {
    *d++ = *s++;
  }
}

// Stubs for stdio functions
int printf(const char *format, ...) {
  (void)format;
  return 0;
}

int putchar(int c) {
  (void)c;
  return 0;
}
// Linker stubs for libgcc / toolchain
void __main(void) {} // Stub for constructor caller
int atexit(void (*function)(void)) { return 0; }

// Empty constructor/destructor lists
// Some toolchains expect these symbols when libgcc is linked
void *__CTOR_LIST__[1] = {(void *)-1};
void *__DTOR_LIST__[1] = {(void *)-1};
