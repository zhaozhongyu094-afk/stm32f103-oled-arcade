#include "gfx.h"
#include <string.h>
static uint8_t frame[FRAME_BYTES];
const uint8_t *gfx_data(void) {
    return frame;
}
void gfx_clear(void) {
    memset(frame, 0, sizeof frame);
}
void gfx_rect(int x, int y, int w, int h, int on) {
    int right = x + w, bottom = y + h;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (right > OLED_W)
        right = OLED_W;
    if (bottom > OLED_H)
        bottom = OLED_H;
    for (int row = y; row < bottom; row++) {
        uint8_t bit = (uint8_t)(1u << (row & 7));
        for (int col = x; col < right; col++) {
            uint8_t *p = &frame[(row / 8) * OLED_W + col];
            if (on)
                *p |= bit;
            else
                *p &= (uint8_t)~bit;
        }
    }
}
void gfx_box(int x, int y, int w, int h) {
    gfx_rect(x, y, w, 1, 1);
    gfx_rect(x, y + h - 1, w, 1, 1);
    gfx_rect(x, y, 1, h, 1);
    gfx_rect(x + w - 1, y, 1, h, 1);
}
// Five rows, three bits per row. Glyphs remain readable in one OLED page.
static const char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-:/+>";
static const uint8_t rows[][5] = {
    {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {6, 1, 7, 4, 7}, {6, 1, 6, 1, 6}, {5, 5, 7, 1, 1},
    {7, 4, 6, 1, 6}, {3, 4, 7, 5, 7}, {7, 1, 2, 2, 2}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 6},
    {2, 5, 7, 5, 5}, {6, 5, 6, 5, 6}, {3, 4, 4, 4, 3}, {6, 5, 5, 5, 6}, {7, 4, 6, 4, 7},
    {7, 4, 6, 4, 4}, {3, 4, 5, 5, 3}, {5, 5, 7, 5, 5}, {7, 2, 2, 2, 7}, {1, 1, 1, 5, 2},
    {5, 5, 6, 5, 5}, {4, 4, 4, 4, 7}, {5, 7, 7, 5, 5}, {5, 7, 7, 7, 5}, {2, 5, 5, 5, 2},
    {6, 5, 6, 4, 4}, {2, 5, 5, 7, 3}, {6, 5, 6, 5, 5}, {3, 4, 2, 1, 6}, {7, 2, 2, 2, 2},
    {5, 5, 5, 5, 7}, {5, 5, 5, 5, 2}, {5, 5, 7, 7, 5}, {5, 5, 2, 5, 5}, {5, 5, 2, 2, 2},
    {7, 1, 2, 4, 7}, {0, 0, 7, 0, 0}, {0, 2, 0, 2, 0}, {1, 1, 2, 4, 4}, {0, 2, 7, 2, 0},
    {4, 2, 1, 2, 4}};
void gfx_sprite(int x, int y, const uint8_t *data, int w, int h, int flip) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            if (data[j] & (1u << (w - 1 - i)))
                gfx_rect(x + (flip ? w - 1 - i : i), y + j, 1, 1, 1);
}
void gfx_text(int x, int y, const char *s) {
    for (; *s; s++, x += 4) {
        const char *pos = strchr(alphabet, *s);
        if (pos)
            gfx_sprite(x, y, rows[pos - alphabet], 3, 5, 0);
    }
}
void gfx_number(int x, int y, unsigned n) {
    char buf[12];
    char *p = buf + sizeof(buf) - 1;
    *p = 0;
    do {
        *--p = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    gfx_text(x, y, p);
}
void gfx_panel(const char *title, const char *hint) {
    gfx_rect(5, 17, 118, 30, 0);
    gfx_box(5, 17, 118, 30);
    gfx_text(64 - (int)strlen(title) * 2, 23, title);
    gfx_text(64 - (int)strlen(hint) * 2, 35, hint);
}
