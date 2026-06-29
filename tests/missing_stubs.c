// tests/missing_stubs.c
#include "../src/framework.h"
#include "../src/main.h"

// Stub for drawWonScreen
void drawWonScreen(const RendererParameters *params, const Textures *textures) {
    (void)params;
    (void)textures;
}

// Stub for drawLostScreen
void drawLostScreen(const RendererParameters *params, const Textures *textures) {
    (void)params;
    (void)textures;
}

// Stub for drawSimpleNumber
void drawSimpleNumber(char num, int x, int y, const RendererParameters *params, int scale) {
    (void)num;
    (void)x;
    (void)y;
    (void)params;
    (void)scale;
}

// Stub for drawPauseScreen if needed
void drawPauseScreen(const RendererParameters *params, const Textures *textures) {
    (void)params;
    (void)textures;
}
