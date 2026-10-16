/* Rastera runtime: C code linked into every compiled Rastera program.
 *
 * The process entry point lives here, not in generated code: the C `main`
 * calls the user's `@main()`, which the compiler emits as `rs_main`. This
 * gives the runtime a place to set up and tear down (e.g. flush output,
 * release the canvas) around the user program. Built-ins are added here as
 * `rt_<name>` functions. */
#include <stdint.h>
#include <stdio.h>

int32_t rs_main(void);

int32_t rt_print(int32_t val) {
    printf("%d\n", (int)val);
    return 0;
}

int main(void) {
    return (int)rs_main();
}
