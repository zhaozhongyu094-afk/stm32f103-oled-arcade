#ifndef GFX_H
#define GFX_H
#include <stdint.h>
enum { OLED_W = 128, OLED_H = 64, FRAME_BYTES = 1024 };
const uint8_t *gfx_data(void);
void gfx_clear(void);
void gfx_rect(int x, int y, int w, int h, int on);
void gfx_box(int x, int y, int w, int h);
void gfx_text(int x, int y, const char *s);
void gfx_number(int x, int y, unsigned n);
void gfx_sprite(int x, int y, const uint8_t *rows, int width, int height, int flip);
void gfx_panel(const char *title, const char *hint);
#endif
