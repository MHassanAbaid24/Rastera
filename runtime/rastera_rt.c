/* Rastera runtime: C code linked into every compiled Rastera program.
 *
 * The process entry point lives here, not in generated code: the C `main`
 * calls the user's `@main()`, which the compiler emits as `rs_main`. This
 * gives the runtime a place to set up and tear down (e.g. flush output,
 * release the canvas) around the user program. Built-ins are added here as
 * `rt_<name>` functions. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t rs_main(void);

/* rt_alloc(n): allocate space for n i32 elements and return a pointer.
 * n < 0 is a runtime error (spec §8). arr(0) allocates a zero-length array. */
int32_t* rt_alloc(int32_t n) {
    if (n < 0) {
        fprintf(stderr, "rastera: runtime error: negative array size\n");
        exit(2);
    }
    size_t bytes = (size_t)n * 4;
    if (bytes == 0) bytes = 1;  /* malloc(0) is implementation-defined */
    return (int32_t*)malloc(bytes);
}

int32_t rt_print(int32_t val) {
    printf("%d\n", (int)val);
    return 0;
}

int main(void) {
    return (int)rs_main();
}
