#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <SDL2/SDL.h>
#include <stdlib.h>


#include "../src/main.h"
#include "../src/framework.h"
#include "test_helpers.h"  // Add this line


// Helpers to build a simple empty level of HEIGHTxWIDTH with borders and a pacgum at (29,26)
static char** alloc_level(void)
{
    char** lvl = (char**)malloc(sizeof(char*) * HEIGHT);
    for (int y = 0; y < HEIGHT; y++)
    {
        lvl[y] = malloc(sizeof(char) * WIDTH);
    }
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            char c = ' ';
            if (y == 0 || y == 30 || x == 0 || x == 27) c = 'H';
            lvl[y][x] = c;
        }
    }
    // place a Pacgum to prevent win
    lvl[29][26] = '.';
    return lvl;
}

static void free_level(char** lvl)
{
    if (!lvl) return;
    for (int y = 0; y < HEIGHT; y++) { free(lvl[y]); }
    free(lvl);
}

/* Setup : alloue et initialise le contexte */
static int setup(void** state)
{
    fw_test_setup();
    char** lvl = alloc_level();
    *state = lvl;
    return 0;
}

/* Teardown : libère le contexte */
static int teardown(void** state)
{
    char** lvl = *(char***)state;
    free_level(lvl);
    fw_test_teardown();
    return 0;
}

// ========== Deplacement ==========

static void test_pacman_followUserKey(void** state)
{
    char** lvl = *(char***)state;
    lvl[1][1] = 'O'; // pacman
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    lvl[29][4] = 'B';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, SDLK_RIGHT, SDLK_DOWN, SDLK_DOWN, SDLK_LEFT, SDLK_UP, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    int x, y;
    fw_test_get_pacman(&x, &y);
    assert_int_equal(y, 2);
    assert_int_equal(x, 2);
}

// Pac-Man continue with neutral input (SDLK_0 -> 0 in our mock)
static void test_pacman_continueOnNeutral(void** state)
{
    char** lvl = *(char***)state;
    lvl[1][1] = 'O';
    lvl[1][2] = ' ';
    lvl[1][3] = ' ';
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    lvl[29][4] = 'B';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, 0, 0, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    int x, y;
    fw_test_get_pacman(&x, &y);
    assert_int_equal(y, 1);
    assert_int_equal(x, 5);
}

static void test_pacman_stopOnWall(void** state)
{
    char** lvl = *(char***)state;
    lvl[1][1] = 'O';
    lvl[1][3] = 'H'; // wall on the right
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    lvl[29][4] = 'B';
    const int inputs[] = {SDLK_RIGHT, 0, 0, 0, SDLK_ESCAPE};
    fw_test_set_level(lvl);
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    int x, y;
    fw_test_get_pacman(&x, &y);
    assert_int_equal(y, 1);
    assert_int_equal(x, 2);
}

// Commande mémorisée: up is blocked then right, then neutral
static void test_pacman_memorizeLastCommand(void** state)
{
    char** lvl = *(char***)state;
    lvl[2][1] = 'O';
    lvl[1][1] = 'H'; // wall above
    lvl[1][2] = 'H';
    lvl[1][3] = 'H';
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    lvl[29][4] = 'B';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, SDLK_UP, 0, 0, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    int x, y;
    fw_test_get_pacman(&x, &y);
    assert_int_equal(y, 1);
    assert_int_equal(x, 4);
}

// Pac-gum disappears when eaten
static void test_pacman_eatPacgum(void** state)
{
    char** lvl = *(char***)state;
    lvl[1][1] = 'O';
    lvl[1][2] = '.'; // pac-gum to the right
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    lvl[29][4] = 'B';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    char** drawn_level = fw_test_get_drawn_level();
    assert_int_equal(drawn_level[1][2], ' ');
}

// Super pac-gum: ghosts turn blue when eaten
static void test_pacman_eatSuperPacgum_ghostsTurnBlue(void** state)
{
    char** lvl = *(char***)state;
    // Place Pac-Man and a super pac-gum
    lvl[1][1] = 'O';
    lvl[1][2] = 'G';
    // add ghosts
    lvl[13][1] = 'C';
    lvl[15][1] = 'P';
    lvl[17][1] = 'I';
    lvl[19][1] = 'B';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    char** drawn = fw_test_get_drawn_level();
    assert_int_equal(drawn[1][2], ' '); // super pac-gum consumed
    int blue = fw_test_get_blue_draws();
    assert_int_equal(blue, 4); // four blue ghost draws in the frame
}

static void test_quitOnEscape(void** state)
{
    char** lvl = *(char***)state;
    lvl[1][1] = 'O';
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    lvl[29][4] = 'B';
    const int inputs[] = {SDLK_ESCAPE};
    fw_test_set_level(lvl);
    fw_test_set_inputs(inputs, 1);

    int res = gameLoop();

    int round = fw_test_get_round_count();
    assert_int_equal(res, 0);
    assert_int_equal(round, 0);
}

// Fin du jeu: collision fantôme
static void test_pacman_crossPath_endGame(void** state)
{
    char** lvl = *(char***)state;
    // Place Pac-Man and a ghost (Blinky) adjacent
    lvl[1][1] = 'O';
    lvl[1][2] = 'C';
    lvl[2][1] = 'H'; // wall around
    lvl[2][2] = 'H';
    lvl[2][3] = 'H';
    lvl[1][3] = 'H';
    // set ghosts away
    lvl[29][1] = 'B';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, 0, SDLK_ESCAPE}; // move onto ghost then ensure we exit if not already
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    int res = gameLoop();

    // After first frame, the game ended and return negative score
    assert_true(res < 0);
    int round = fw_test_get_round_count();
    assert_int_equal(round, 1);
}

// Fin du jeu: collision fantôme
static void test_pacman_encounterGhost_endGame(void** state)
{
    char** lvl = *(char***)state;
    // Place Pac-Man and a ghost (Blinky) adjacent
    lvl[1][1] = 'O';
    lvl[1][3] = 'C';
    lvl[2][1] = 'H'; // wall around
    lvl[2][2] = 'H';
    lvl[2][3] = 'H';
    lvl[2][4] = 'H';
    lvl[1][4] = 'H';
    // set ghosts away
    lvl[29][1] = 'B';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, 0, 0, 0, 0, SDLK_ESCAPE}; // move onto ghost then ensure we exit if not already
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    int res = gameLoop();

    // After first frame, the game ended and return negative score
    assert_true(res < 0);
    int round = fw_test_get_round_count();
    assert_true(round <= 2);
}

void makesParallelsWalls(char*** lvl)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (y % 2 == 0) (*lvl)[y][x] = 'H';
        }
    }
}

// Fantômes ne traversent pas les murs
static void test_ghosts_dontPassThroughWalls(void** state)
{
    char** lvl = *(char***)state;

    makesParallelsWalls(&lvl);

    // Place Pac-Man in the first line
    lvl[1][1] = 'O';
    // Place a ghost in each line
    lvl[3][1] = 'C';
    lvl[5][1] = 'P';
    lvl[7][1] = 'I';
    lvl[9][1] = 'B';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, 0, 0, 0, 0, 0, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    // Check ghost moved and didn't go through wall
    int gx, gy;
    fw_test_get_ghost(0, &gx, &gy);
    assert_int_equal(gx, 8);
    assert_int_equal(gy, 3);
    fw_test_get_ghost(1, &gx, &gy);
    assert_int_equal(gx, 8);
    assert_int_equal(gy, 5);
    fw_test_get_ghost(2, &gx, &gy);
    assert_int_equal(gx, 8);
    assert_int_equal(gy, 7);
    fw_test_get_ghost(3, &gx, &gy);
    assert_int_equal(gx, 8);
    assert_int_equal(gy, 9);
}

void drawAC(char*** lvl, int i, char entity)
{
    (*lvl)[1 + i][4] = 'H';
    (*lvl)[1 + i][3] = entity;
    (*lvl)[2 + i][1] = ' ';
    (*lvl)[3 + i][4] = 'H';
}

void init_C_shaped_corridors(void** state)
{
    char** lvl = *(char***)state;

    makesParallelsWalls(&lvl);

    // Place a ghost in each C
    drawAC(&lvl, 0, 'C');
    drawAC(&lvl, 4, 'P');
    drawAC(&lvl, 8, 'I');
    drawAC(&lvl, 12, 'B');
    drawAC(&lvl, 16, 'O');
    fw_test_set_level(lvl);
}

// Fantômes ne font pas demi-tour
static void test_ghosts_noUturn(void** state)
{
    init_C_shaped_corridors(state);
    const int inputs[] = {SDLK_RIGHT, 0, 0, 0, 0, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    // Check ghost moved and didn't go through wall
    int gx, gy;
    fw_test_get_ghost(0, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 3);
    fw_test_get_ghost(1, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 7);
    fw_test_get_ghost(2, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 11);
    fw_test_get_ghost(3, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 15);
}

// Fantômes ne traversent pas les murs, ne font pas demi-tour, ne s'arrêtent jamais
static void test_pacmanAndGhosts_wraparound(void** state)
{
    char** lvl = *(char***)state;

    makesParallelsWalls(&lvl);

    // Place Pac-Man in the first line
    lvl[1][25] = 'O';
    lvl[1][24] = 'H';
    lvl[1][0] = ' ';
    lvl[1][27] = ' ';
    // Place a ghost in each line
    lvl[3][25] = 'C';
    lvl[3][24] = 'H';
    lvl[3][0] = ' ';
    lvl[3][27] = ' ';
    lvl[5][25] = 'P';
    lvl[5][24] = 'H';
    lvl[5][0] = ' ';
    lvl[5][27] = ' ';
    lvl[7][25] = 'I';
    lvl[7][24] = 'H';
    lvl[7][0] = ' ';
    lvl[7][27] = ' ';
    lvl[9][25] = 'B';
    lvl[9][24] = 'H';
    lvl[9][0] = ' ';
    lvl[9][27] = ' ';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, 0, 0, 0, 0, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    // Check Pacman and ghosts wraparound
    int gx, gy;
    fw_test_get_pacman(&gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 1);
    fw_test_get_ghost(0, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 3);
    fw_test_get_ghost(1, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 5);
    fw_test_get_ghost(2, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 7);
    fw_test_get_ghost(3, &gx, &gy);
    assert_int_equal(gx, 3);
    assert_int_equal(gy, 9);
}

// Victoire quand il n'y a plus de pacgum
static void test_pacman_eatLastPacgum_victory(void** state)
{
    char** lvl = *(char***)state;
    lvl[29][26] = ' '; // remove the only pac-gum
    lvl[5][5] = 'O';
    lvl[5][6] = '.'; // last pac-gum
    lvl[5][7] = '.'; // last pac-gum
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';
    lvl[29][4] = 'B';
    fw_test_set_level(lvl);
    const int inputs[] = {SDLK_RIGHT, 0, 0, 0, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    int res = gameLoop();

    assert_true(res > 0);
    int round = fw_test_get_round_count();
    assert_int_equal(round, 2);
}

// L'image suit la direction (Pacman et fantômes)
static void test_sprites_followDirection_left(void** state)
{
    init_C_shaped_corridors(state);
    const int inputs[] = {SDLK_LEFT, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    float a = fw_test_get_pacman_angle();
    assert_int_equal((int)a, 0);
    char dir = fw_test_get_ghost_last_dir(0);
    assert_true(dir == 'L');
    fw_test_get_ghost_last_dir(1);
    assert_true(dir == 'L');
    fw_test_get_ghost_last_dir(2);
    assert_true(dir == 'L');
    fw_test_get_ghost_last_dir(3);
    assert_true(dir == 'L');
}

static void test_sprites_followDirection_down(void** state)
{
    init_C_shaped_corridors(state);
    const int inputs[] = {SDLK_LEFT, 0, SDLK_DOWN, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    float a = fw_test_get_pacman_angle();
    assert_int_equal((int)a, 270);
    char dir = fw_test_get_ghost_last_dir(0);
    assert_true(dir == 'D');
    fw_test_get_ghost_last_dir(1);
    assert_true(dir == 'D');
    fw_test_get_ghost_last_dir(2);
    assert_true(dir == 'D');
    fw_test_get_ghost_last_dir(3);
    assert_true(dir == 'D');
}

static void test_sprites_followDirection_right(void** state)
{
    init_C_shaped_corridors(state);
    const int inputs[] = {SDLK_LEFT, 0, SDLK_DOWN, 0, SDLK_RIGHT, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    float a = fw_test_get_pacman_angle();
    assert_int_equal((int)a, 180);
    char dir = fw_test_get_ghost_last_dir(0);
    assert_true(dir == 'R');
    fw_test_get_ghost_last_dir(1);
    assert_true(dir == 'R');
    fw_test_get_ghost_last_dir(2);
    assert_true(dir == 'R');
    fw_test_get_ghost_last_dir(3);
    assert_true(dir == 'R');
}

static void test_sprites_followDirection_up(void** state)
{
    init_C_shaped_corridors(state);
    const int inputs[] = {SDLK_LEFT, 0, SDLK_DOWN, 0, SDLK_RIGHT, 0, SDLK_LEFT, 0, SDLK_UP, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    gameLoop();

    float a = fw_test_get_pacman_angle();
    assert_int_equal((int)a, 90);
    char dir = fw_test_get_ghost_last_dir(0);
    assert_true(dir == 'U');
    fw_test_get_ghost_last_dir(1);
    assert_true(dir == 'U');
    fw_test_get_ghost_last_dir(2);
    assert_true(dir == 'U');
    fw_test_get_ghost_last_dir(3);
    assert_true(dir == 'U');
}

// Pacman mange un fantôme bleu -> retour centre et couleur normale
static void test_pacman_eats_blue_ghost_then_original_place_and_color_restored(void** state)
{
    char** lvl = *(char***)state;
    makesParallelsWalls(&lvl);
    // Place Pac-Man, super pac-gum, then Blinky in a line
    lvl[1][1] = 'O';
    lvl[1][4] = 'G'; // super pac-gum
    lvl[1][9] = 'B'; // Blinky
    lvl[1][10] = 'H';
    // set ghosts away
    lvl[29][1] = 'C';
    lvl[29][2] = 'P';
    lvl[29][3] = 'I';

    fw_test_set_level(lvl);
    // Frame 1: RIGHT -> eat G (ghosts become blue)
    // Frame 2: RIGHT -> collide with Blinky while blue -> reset to center and frightened=false
    // Frame 3: 0 -> update textures; eaten ghost should no longer be blue -> only 3 blue draws
    const int inputs[] = {SDLK_RIGHT, 0, 0, SDLK_ESCAPE};
    fw_test_set_inputs(inputs, sizeof(inputs) / sizeof(inputs[0]));

    int res = gameLoop();
    assert_int_equal(res, 0); // we quit via escape, not game over

    // On the last frame only 3 blue ghosts should be drawn
    int blue = fw_test_get_blue_draws();

    int a, b;
    fw_test_get_pacman(&a, &b);
    int gx, gy;
    fw_test_get_ghost(3, &gx, &gy);
    assert_int_equal(gx, 8);
    assert_int_equal(gy, 1);

    assert_int_equal(blue, 3);

}

/*
 * Tous ces tests unitaires sont construits comme suit :
 * Arrange  : configurer ce qui doit être testé
 * Act      : Effectuer le travail
 * Assert   : Vérifier le résultat
 *
 *
 * Setup : alloue et initialise le contexte
 * Teardown : libère le contexte
 *
 */

int main(void)
{
    const struct CMUnitTest tests[] = {
        // Deplacement
        cmocka_unit_test_setup_teardown(test_pacman_followUserKey, setup, teardown),
        cmocka_unit_test_setup_teardown(test_pacman_continueOnNeutral, setup, teardown),
        cmocka_unit_test_setup_teardown(test_pacman_stopOnWall, setup, teardown),
        cmocka_unit_test_setup_teardown(test_pacman_memorizeLastCommand, setup, teardown),
        // Affichage logique
        cmocka_unit_test_setup_teardown(test_pacman_eatPacgum, setup, teardown),
        cmocka_unit_test_setup_teardown(test_pacman_eatSuperPacgum_ghostsTurnBlue, setup, teardown),
        cmocka_unit_test_setup_teardown(test_sprites_followDirection_left, setup, teardown),
        cmocka_unit_test_setup_teardown(test_sprites_followDirection_down, setup, teardown),
        cmocka_unit_test_setup_teardown(test_sprites_followDirection_right, setup, teardown),
        cmocka_unit_test_setup_teardown(test_sprites_followDirection_up, setup, teardown),
        // Fin de partie
        cmocka_unit_test_setup_teardown(test_pacman_crossPath_endGame, setup, teardown),
        cmocka_unit_test_setup_teardown(test_pacman_encounterGhost_endGame, setup, teardown),
        // Déplacement fantômes
        cmocka_unit_test_setup_teardown(test_ghosts_dontPassThroughWalls, setup, teardown),
        cmocka_unit_test_setup_teardown(test_ghosts_noUturn, setup, teardown),
        cmocka_unit_test_setup_teardown(test_pacmanAndGhosts_wraparound, setup, teardown),
        // Fin de jeu
        cmocka_unit_test_setup_teardown(test_pacman_eatLastPacgum_victory, setup, teardown),
        cmocka_unit_test_setup_teardown(test_quitOnEscape, setup, teardown),

        // WIP
        //cmocka_unit_test_setup_teardown(test_pacman_eats_blue_ghost_then_original_place_and_color_restored, setup, teardown),

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
