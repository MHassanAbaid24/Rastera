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

void rt_panic(const char* message) {
    fflush(stdout);
    fprintf(stderr, "rastera: runtime error: %s\n", message);
    exit(2);
}

/* rt_alloc(n): reserve a length header followed by n i32 elements. */
int32_t* rt_alloc(int32_t n) {
    if (n < 0) rt_panic("negative array size");
    int32_t* storage = (int32_t*)malloc(((size_t)n + 1) * sizeof(*storage));
    storage[0] = n;
    return storage + 1;
}

int32_t rt_print(int32_t val) {
    printf("%d\n", (int)val);
    return 0;
}

int main(void) {
    return (int)rs_main();
}
