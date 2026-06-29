#include "../src/firstLevel.h"
#include "../src/framework.h"
#include <stdlib.h>
#include <string.h>

#include "test_helpers.h"  // Add this line


#ifdef PACMAN_TESTS

static char** clone_or_default()
{

    char **tab = malloc(sizeof(char*) * 31);
    for (int i = 0; i < 31; i++)
    {
        tab[i] = (char*)malloc(sizeof(char) * 28);
    }

    char** src = fw_test_get_level();
    if (src)
    {
        for (int i = 0; i < 31; i++)
        {
            memcpy(tab[i], src[i], 28);
        }
    }
    else
    {
        // default empty level with borders as walls
        for (int y = 0; y < 31; y++)
        {
            for (int x = 0; x < 28; x++)
            {
                char c = ' ';
                if (y == 0 || y == 30 || x == 0 || x == 27) c = 'H';
                tab[y][x] = c;
            }
        }
        // place Pacman somewhere free
        tab[1][1] = 'O';
    }
    return tab;
}

char** loadFirstLevel()
{
    return clone_or_default();
}

#endif // PACMAN_TESTS
