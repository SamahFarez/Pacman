// tests/test_helpers.h
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <SDL2/SDL.h>

// Test helper functions
void fw_test_setup(void);
void fw_test_set_inputs(const int* keys, int count);
void fw_test_get_pacman(int* x, int* y);
void fw_test_get_ghost(int index, int* x, int* y);
float fw_test_get_pacman_angle(void);
char fw_test_get_ghost_last_dir(int index);
void fw_test_set_level(char** level);
char** fw_test_get_level(void);
char** fw_test_get_drawn_level(void);
int fw_test_get_draw_count(void);
int fw_test_get_round_count(void);
int fw_test_get_blue_draws(void);
void fw_test_teardown(void);

#endif
