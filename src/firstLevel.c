#include "firstLevel.h"
#include <string.h>
#include <stdlib.h>

// Chargement du premier niveau sous forme d'un tableau
char ** loadFirstLevel()
{
    char **tab = malloc(sizeof(char *) * 31);
    for (int i = 0; i < 31; i++)
    {
        tab[i] = malloc(sizeof(char) * 28);
    }
    strcpy( tab[0],  "HHHHHHHHHHHHHHHHHHHHHHHHHHHH");
    strcpy( tab[1],  "H............HH............H");
    strcpy( tab[2],  "H.HHHH.HHHHH.HH.HHHHH.HHHH.H");
    strcpy( tab[3],  "HGHHHH.HHHHH.HH.HHHHH.HHHHGH");
    strcpy( tab[4],  "H.HHHH.HHHHH.HH.HHHHH.HHHH.H");
    strcpy( tab[5],  "H..........................H");
    strcpy( tab[6],  "H.HHHH.HH.HHHHHHHH.HH.HHHH.H");
    strcpy( tab[7],  "H.HHHH.HH.HHHHHHHH.HH.HHHH.H");
    strcpy( tab[8],  "H......HH....HH....HH......H");
    strcpy( tab[9],  "HHHHHH.HHHHH HH HHHHH.HHHHHH");
    strcpy( tab[10], "HHHHHH.HHHHH HH HHHHH.HHHHHH");
    strcpy( tab[11], "HHHHHH.HH P C    I HH.HHHHHH");
    strcpy( tab[12], "HHHHHH.HH HHH  HHH HH.HHHHHH");
    strcpy( tab[13], "HHHHHH.HH H      H HH.HHHHHH");
    strcpy( tab[14], "      .   H  B   H   .      ");
    strcpy( tab[15], "HHHHHH.HH H      H HH.HHHHHH");
    strcpy( tab[16], "HHHHHH.HH HHHHHHHH HH.HHHHHH");
    strcpy( tab[17], "HHHHHH.HH          HH.HHHHHH");
    strcpy( tab[18], "HHHHHH.HH HHHHHHHH HH.HHHHHH");
    strcpy( tab[19], "HHHHHH.HH HHHHHHHH HH.HHHHHH");
    strcpy( tab[20], "H............HH............H");
    strcpy( tab[21], "H.HHHH.HHHHH.HH.HHHHH.HHHH.H");
    strcpy( tab[22], "H.HHHH.HHHHH.HH.HHHHH.HHHH.H");
    strcpy( tab[23], "HG..HH....... O.......HH..GH");
    strcpy( tab[24], "HHH.HH.HH.HHHHHHHH.HH.HH.HHH");
    strcpy( tab[25], "HHH.HH.HH.HHHHHHHH.HH.HH.HHH");
    strcpy( tab[26], "H......HH.HHHHHHHH.HH......H");
    strcpy( tab[27], "H.HHHHHHH.HHHHHHHH.HHHHHHH.H");
    strcpy( tab[28], "H.HHHHHHH.HHHHHHHH.HHHHHHH.H");
    strcpy( tab[29], "H..........................H");
    strcpy( tab[30], "HHHHHHHHHHHHHHHHHHHHHHHHHHHH");

    return tab;
}


