#include "../src/framework.h"
#include <stdlib.h>
#include <string.h>

#ifdef PACMAN_TESTS

// ---- Internal mock state ----
static RendererParameters g_params;
static Textures g_textures;

static int* g_input_queue = NULL;
static int g_input_count = 0;
static int g_input_index = 0;

static int g_draw_count = 0;
static int g_round_count = 0;
static int g_blue_draws = 0;
static int g_pacman_x = -1, g_pacman_y = -1;
static float g_pacman_angle = 0.0f;
static int g_ghost_x[4] = {-1, -1, -1, -1};
static int g_ghost_y[4] = {-1, -1, -1, -1};
static char g_ghost_last_dir[4] = {'?', '?', '?', '?'};

static int g_rand_fixed_enabled = 0;
static int g_rand_fixed_value = 0;

static char** g_drawn_level = NULL;;

static char** g_injected_level = NULL; // Provided by tests (borrowed, not owned)

// Provide unique non-null texture pointers so tests can identify entities
static SDL_Texture* make_dummy_texture(void)
{
    void* p = malloc(1);
    return (SDL_Texture*)p;
}

// ---- Public test helpers ----
void fw_test_setup(void)
{
    free(g_input_queue);
    g_input_queue = NULL;
    g_input_count = 0;
    g_input_index = 0;
    g_draw_count = 0;
    g_round_count = 0;
    g_blue_draws = 0;
    g_pacman_x = g_pacman_y = -1;
    for (int i = 0; i < 4; i++) { g_ghost_x[i] = g_ghost_y[i] = -1; }
    g_rand_fixed_enabled = 0;
    g_rand_fixed_value = 0;
    g_drawn_level = (char**)malloc(sizeof(char*) * HEIGHT);
    for (int y = 0; y < HEIGHT; y++)
    {
        g_drawn_level[y] = malloc(sizeof(char) * WIDTH);
    }
}

void fw_test_set_inputs(const int* keys, int count)
{
    free(g_input_queue);
    g_input_queue = NULL;
    g_input_count = 0;
    g_input_index = 0;
    if (count > 0)
    {
        g_input_queue = (int*)malloc(sizeof(int) * count);
        memcpy(g_input_queue, keys, sizeof(int) * count);
        g_input_count = count;
        g_input_index = 0;
    }
}

void fw_test_get_pacman(int* x, int* y)
{
    if (x) *x = g_pacman_x;
    if (y) *y = g_pacman_y;
}

void fw_test_get_ghost(int index, int* x, int* y)
{
    if (index >= 0 && index < 4)
    {
        if (x) *x = g_ghost_x[index];
        if (y) *y = g_ghost_y[index];
    }
}

float fw_test_get_pacman_angle(void) { return g_pacman_angle; }

char fw_test_get_ghost_last_dir(int index)
{
    if (index >= 0 && index < 4) return g_ghost_last_dir[index];
    return '?';
}

void fw_test_set_level(char** level) { g_injected_level = level; }
char** fw_test_get_level(void) { return g_injected_level; }
char** fw_test_get_drawn_level(void) { return g_drawn_level; }
int fw_test_get_draw_count(void) { return g_draw_count; }
int fw_test_get_round_count(void) { return g_round_count; }
int fw_test_get_blue_draws(void) { return g_blue_draws; }

// ---- Framework API (mocked) ----

// Mock RNG control
void rng_seed(unsigned int seed)
{
    (void)seed; /* no-op when fixed enabled */
}

int rng_rand(int max)
{
    if (max <= 0) return 0;
    if (g_rand_fixed_enabled)
    {
        int v = g_rand_fixed_value % max;
        if (v < 0) v += max;
        return v;
    }
    // fallback to deterministic but simple LCG so it is reproducible
    static unsigned int s = 123456789u;
    s = 1103515245u * s + 12345u;
    return (int)((s >> 16) % (unsigned int)max);
}

void init(RendererParameters* params, Textures* textures, int width, int height, int fps)
{
    (void)width;
    (void)height;
    (void)fps;
    memset(params, 0, sizeof(*params));
    *params = g_params; // not used but kept

    // create distinct dummy textures
    textures->texturePacman = make_dummy_texture();
    textures->textureWall = make_dummy_texture();
    textures->textureDot = make_dummy_texture();
    textures->textureSuperPacgum = make_dummy_texture();
    textures->textureBlinky = make_dummy_texture();
    textures->textureBlinkyL = make_dummy_texture();
    textures->textureBlinkyU = make_dummy_texture();
    textures->textureBlinkyD = make_dummy_texture();
    textures->texturePinky = make_dummy_texture();
    textures->texturePinkyL = make_dummy_texture();
    textures->texturePinkyU = make_dummy_texture();
    textures->texturePinkyD = make_dummy_texture();
    textures->textureInky = make_dummy_texture();
    textures->textureInkyL = make_dummy_texture();
    textures->textureInkyU = make_dummy_texture();
    textures->textureInkyD = make_dummy_texture();
    textures->textureClyde = make_dummy_texture();
    textures->textureClydeL = make_dummy_texture();
    textures->textureClydeU = make_dummy_texture();
    textures->textureClydeD = make_dummy_texture();
    textures->textureBLue = make_dummy_texture();
    textures->cellSize = 14;

    g_textures = *textures; // keep a copy for identification
}

void initWindowed(RendererParameters* params, Textures* textures)
{
    init(params, textures, 0, 0, 60);
}

int getInput(void)
{
    if (g_input_index < g_input_count)
    {
        int key = g_input_queue[g_input_index++];
        // SDLK_ESCAPE is 27 in SDL
        if (key == SDLK_ESCAPE) {
            // When escape is pressed, also simulate quit
            return key;
        }
        return key;
    }
    // When no more inputs, return SDLK_ESCAPE to quit
    return SDLK_ESCAPE; // Force quit when input exhausted
}

void update(RendererParameters* params)
{
    (void)params; /* no delay */
    g_round_count++;
}

void drawSpriteOnGrid(SDL_Texture* texture, int x, int y, float angle, const RendererParameters* params)
{
    (void)params;
    g_draw_count++;
    if (texture == g_textures.texturePacman)
    {
        g_pacman_x = x;
        g_pacman_y = y;
        g_pacman_angle = angle;
    }
    // Blue texture: counts how many ghost draws in frightened state
    else if (texture == g_textures.textureBLue)
    {
        g_blue_draws++;
    }
    // Clyde (any directional texture)
    else if (texture == g_textures.textureClyde || texture == g_textures.textureClydeL || texture == g_textures.
        textureClydeU || texture == g_textures.textureClydeD)
    {
        g_ghost_x[0] = x;
        g_ghost_y[0] = y;
        g_ghost_last_dir[0] = (texture == g_textures.textureClydeL
                                   ? 'L'
                                   : texture == g_textures.textureClydeU
                                   ? 'U'
                                   : texture == g_textures.textureClydeD
                                   ? 'D'
                                   : 'R');
    }
    // Pinky
    else if (texture == g_textures.texturePinky || texture == g_textures.texturePinkyL || texture == g_textures.
        texturePinkyU || texture == g_textures.texturePinkyD)
    {
        g_ghost_x[1] = x;
        g_ghost_y[1] = y;
        g_ghost_last_dir[1] = (texture == g_textures.texturePinkyL
                                   ? 'L'
                                   : texture == g_textures.texturePinkyU
                                   ? 'U'
                                   : texture == g_textures.texturePinkyD
                                   ? 'D'
                                   : 'R');
    }
    // Inky
    else if (texture == g_textures.textureInky || texture == g_textures.textureInkyL || texture == g_textures.
        textureInkyU || texture == g_textures.textureInkyD)
    {
        g_ghost_x[2] = x;
        g_ghost_y[2] = y;
        g_ghost_last_dir[2] = (texture == g_textures.textureInkyL
                                   ? 'L'
                                   : texture == g_textures.textureInkyU
                                   ? 'U'
                                   : texture == g_textures.textureInkyD
                                   ? 'D'
                                   : 'R');
    }
    // Blinky
    else if (texture == g_textures.textureBlinky || texture == g_textures.textureBlinkyL || texture == g_textures.
        textureBlinkyU || texture == g_textures.textureBlinkyD)
    {
        g_ghost_x[3] = x;
        g_ghost_y[3] = y;
        g_ghost_last_dir[3] = (texture == g_textures.textureBlinkyL
                                   ? 'L'
                                   : texture == g_textures.textureBlinkyU
                                   ? 'U'
                                   : texture == g_textures.textureBlinkyD
                                   ? 'D'
                                   : 'R');
    }
    // Note: if ghosts are blue, all share the same texture; we don't update a specific ghost then.
}

void drawSprite(SDL_Texture* texture, int x, int y, float angle, const RendererParameters* params)
{
    drawSpriteOnGrid(texture, x, y, angle, params);
}

void drawSpriteAlpha(SDL_Texture* texture, int x, int y, int w, int h, float angle, float alpha,
                     const RendererParameters* params)
{
    (void)w;
    (void)h;
    (void)alpha;
    drawSpriteOnGrid(texture, x, y, angle, params);
}

void drawLevel(char** level, const RendererParameters* params, const Textures* textures)
{
    (void)params;
    (void)textures;
    g_blue_draws = 0;
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            g_drawn_level[i][j] = level[i][j];
        }
    }
}

SDL_Texture* getTexture(char* imageName, const RendererParameters* params)
{
    (void)imageName;
    (void)params;
    return make_dummy_texture();
}

int getRandomNumber()
{
    // Renvoie 1 pour casser l'aléatoire
    // Bloque tous les tests de type getRandomNumber() % X == 0
    return 1;
}

void cleanUp(const RendererParameters* params, char** level)
{

    // free textures
#define FREE_TEX(t) do{ if ((t)) { free((void*)(t)); (t)=NULL; } }while(0)
    FREE_TEX(g_textures.texturePacman);
    FREE_TEX(g_textures.textureWall);
    FREE_TEX(g_textures.textureDot);
    FREE_TEX(g_textures.textureSuperPacgum);
    FREE_TEX(g_textures.textureBlinky);
    FREE_TEX(g_textures.textureBlinkyL);
    FREE_TEX(g_textures.textureBlinkyU);
    FREE_TEX(g_textures.textureBlinkyD);
    FREE_TEX(g_textures.texturePinky);
    FREE_TEX(g_textures.texturePinkyL);
    FREE_TEX(g_textures.texturePinkyU);
    FREE_TEX(g_textures.texturePinkyD);
    FREE_TEX(g_textures.textureInky);
    FREE_TEX(g_textures.textureInkyL);
    FREE_TEX(g_textures.textureInkyU);
    FREE_TEX(g_textures.textureInkyD);
    FREE_TEX(g_textures.textureClyde);
    FREE_TEX(g_textures.textureClydeL);
    FREE_TEX(g_textures.textureClydeU);
    FREE_TEX(g_textures.textureClydeD);
    FREE_TEX(g_textures.textureBLue);
}

void fw_test_teardown(void)
{
    if (g_drawn_level)
    {
        for (int i = 0; i < HEIGHT; i++)
        {
            if (g_drawn_level[i]) free(g_drawn_level[i]);
        }
        free(g_drawn_level);
        g_drawn_level = NULL;
    }

    free(g_input_queue);
    g_input_queue = NULL;
}


// Add these missing functions that main.c needs
void drawSimpleText(const char *text, int x, int y, const RendererParameters *params, int scale)
{
    (void)text;
    (void)x;
    (void)y;
    (void)params;
    (void)scale;
}

void drawGameOverScreen(const RendererParameters *params, const Textures *textures)
{
    (void)params;
    (void)textures;
}

void drawWonScreen(const RendererParameters *params, const Textures *textures)
{
    (void)params;
    (void)textures;
}

void drawLostScreen(const RendererParameters *params, const Textures *textures)
{
    (void)params;
    (void)textures;
}

void drawSimpleNumber(char num, int x, int y, const RendererParameters *params, int scale)
{
    (void)num;
    (void)x;
    (void)y;
    (void)params;
    (void)scale;
}

void drawPauseScreen(const RendererParameters *params, const Textures *textures)
{
    (void)params;
    (void)textures;
}

void drawCenteredText(const char *text, int y, const RendererParameters *params, int scale)
{
    (void)text;
    (void)y;
    (void)params;
    (void)scale;
}

#endif // PACMAN_TESTS
