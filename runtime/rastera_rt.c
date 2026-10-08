/* Rastera runtime: C code linked into every compiled Rastera program.
 *
 * The process entry point lives here, not in generated code: the C `main`
 * calls the user's `@main()`, which the compiler emits as `rs_main`. This
 * gives the runtime a place to set up and tear down (e.g. flush output,
 * release the canvas) around the user program. Built-ins are added here as
 * `rt_<name>` functions. */
#include <stdint.h>

int32_t rs_main(void);

int main(void) {
    return (int)rs_main();
}
