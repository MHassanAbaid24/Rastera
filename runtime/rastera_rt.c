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

#define CANVAS_MAX 4096

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

/* The canvas: one w*h image of packed RGB bytes, row-major from the top-left.
 * There is at most one; `canvas` replaces it and exit releases it. */
static int32_t canvas_w;
static int32_t canvas_h;
static uint8_t* canvas_px;

static void canvas_free(void) {
    free(canvas_px);
    canvas_px = NULL;
}

static void require_canvas(void) {
    if (canvas_px == NULL) rt_panic("no canvas");
}

/* Store the low 24 bits of c (0xRRGGBB) at (x, y) if it lies inside the canvas. */
static void put_pixel(int64_t x, int64_t y, int32_t c) {
    if (x < 0 || y < 0 || x >= canvas_w || y >= canvas_h) return;
    uint8_t* p = canvas_px + ((size_t)y * (size_t)canvas_w + (size_t)x) * 3;
    p[0] = (uint8_t)((uint32_t)c >> 16);
    p[1] = (uint8_t)((uint32_t)c >> 8);
    p[2] = (uint8_t)(uint32_t)c;
}

int32_t rt_canvas(int32_t w, int32_t h) {
    if (w < 1 || w > CANVAS_MAX || h < 1 || h > CANVAS_MAX) rt_panic("invalid canvas size");
    if (canvas_px == NULL) atexit(canvas_free);
    free(canvas_px);
    canvas_px = (uint8_t*)calloc((size_t)w * (size_t)h, 3);
    if (canvas_px == NULL) rt_panic("out of memory");
    canvas_w = w;
    canvas_h = h;
    return 0;
}

static int32_t clamp_channel(int32_t v) {
    return v < 0 ? 0 : (v > 255 ? 255 : v);
}

int32_t rt_rgb(int32_t r, int32_t g, int32_t b) {
    return (clamp_channel(r) << 16) | (clamp_channel(g) << 8) | clamp_channel(b);
}

int32_t rt_pixel(int32_t x, int32_t y, int32_t c) {
    require_canvas();
    put_pixel(x, y, c);
    return 0;
}

/* Bresenham in 64-bit arithmetic so extreme coordinates cannot overflow. */
int32_t rt_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t c) {
    require_canvas();
    int64_t x = x0, y = y0;
    int64_t dx = x1 > x0 ? (int64_t)x1 - x0 : (int64_t)x0 - x1;
    int64_t dy = y1 > y0 ? (int64_t)y0 - y1 : (int64_t)y1 - y0; /* -|y1 - y0| */
    int64_t sx = x0 < x1 ? 1 : -1;
    int64_t sy = y0 < y1 ? 1 : -1;
    int64_t err = dx + dy;
    for (;;) {
        put_pixel(x, y, c);
        if (x == x1 && y == y1) break;
        int64_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
    return 0;
}

int32_t rt_rect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t c) {
    require_canvas();
    if (w <= 0 || h <= 0) return 0;
    int64_t xa = x < 0 ? 0 : x;
    int64_t ya = y < 0 ? 0 : y;
    int64_t xb = (int64_t)x + w < canvas_w ? (int64_t)x + w : canvas_w;
    int64_t yb = (int64_t)y + h < canvas_h ? (int64_t)y + h : canvas_h;
    for (int64_t j = ya; j < yb; j++)
        for (int64_t i = xa; i < xb; i++) put_pixel(i, j, c);
    return 0;
}

int32_t rt_export(void) {
    require_canvas();
    FILE* f = fopen("out.ppm", "wb");
    if (f == NULL) rt_panic("cannot write out.ppm");
    size_t bytes = (size_t)canvas_w * (size_t)canvas_h * 3;
    int ok = fprintf(f, "P6\n%d %d\n255\n", (int)canvas_w, (int)canvas_h) > 0 &&
             fwrite(canvas_px, 1, bytes, f) == bytes;
    if (fclose(f) != 0) ok = 0;
    if (!ok) rt_panic("cannot write out.ppm");
    return 0;
}

int main(void) {
    return (int)rs_main();
}
