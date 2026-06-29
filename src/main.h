#ifndef MAIN_H
#define MAIN_H
#pragma once
#pragma once

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "framework.h"

#define STARTING_LIVES 3

#define INKY_RANDOMNESS 3  // probabilité 1 sur 3 de mouvement aléatoire
#define BLINKY_RANDOMNESS 4  // probabilité 1 sur 4 de mouvement aléatoire

#define INKY_RANDOMNESS 3
#define BLINKY_RANDOMNESS 4
#define FRIGHTENED_DURATION 100  // Nombre d'images pour la durée du mode frightened


#define SCORE_DOT 10
#define SCORE_SUPER_PACGUM 50
#define SCORE_GHOST_BASE 200  // Score de base quand on mange un fantôme
#define SCORE_GHOST_MULTIPLIER 2  // Multiplicateur pour fantômes consommés consécutivement

typedef struct Coord Coord;
struct Coord
{
    int x; // coordonnée X sur la grille
    int y; // coordonnée Y sur la grille
};

typedef enum EntityName EntityName;
enum EntityName
{
    BLINKY,
    CLYDE,
    INKY,
    PINKY,
    PACMAN
};

typedef enum GhostState GhostState;
enum GhostState
{
    NORMAL,    // état normal
    FRIGHTENED,// effrayé (bleu)
    EATEN      // mangé : retour à la position de départ
};

typedef struct MovingEntity MovingEntity;
struct MovingEntity
{
    Coord pos;   // position courante (case)
    Coord dir;   // direction courante (-1,0,1)
    EntityName name; // identifiant de l'entité (PACMAN/BLINKY/...)
    SDL_Texture *texture; // texture utilisée pour le rendu
    GhostState state; // état du fantôme (NORMAL/FRIGHTENED/EATEN)
    Coord startPos;  // position de départ (utile pour reset/retour)
    int frightenedTimer;  // compteur pour la durée du mode frightened
};

typedef struct {
    int score;
    int ghostMultiplier;  // multiplicateur appliqué lors de fantômes mangés à la suite
    int dotsEaten;
    int ghostsEaten;
    int superPacgumsEaten;
    int lives;
} GameStats;


// Trouve la position initiale de Pacman dans le niveau
void findPacman(char **level, int *px, int *py);

// Teste si toutes les pastilles ont été mangées (condition de victoire)
bool win(const char **level);

// Retourne les directions possibles depuis la position d'une entité (sans murs ni demi-tours)
Coord *getPotentialDirections(const char **level, const MovingEntity *entity, int *nbDir);

// Moteur de déplacement principal d'un fantôme (choisit la stratégie selon le type)
void ghostMove(const char **level, MovingEntity *ghost, const MovingEntity *pacman);

// Vérifie si deux entités se percutent (collision sur la même case ou croisement)
bool checkCollision(const MovingEntity *entity1, const MovingEntity *entity2);

// Teste si un fantôme a une ligne de vue dégagée vers Pacman (ligne/colonne sans murs)
bool hasLineOfSight(const char **level, const MovingEntity *ghost, const MovingEntity *pacman);

// Comportement de Clyde (fantôme rouge) : mouvement essentiellement aléatoire
void clydeMove(const char **level, MovingEntity *clyde, const Coord *pacmanPos);

// Comportement de Pinky (fantôme rose) : tente de se positionner en avant de Pacman
void pinkyMove(const char **level, MovingEntity *pinky, const Coord *pacmanPos);

// Comportement d'Inky (fantôme bleu) : logique combinée dépendant de Pacman et Blinky
void inkyMove(const char **level, MovingEntity *inky, const Coord *pacmanPos);

// Comportement de Blinky (fantôme orange) : poursuit Pacman avec un peu d'aléa
void blinkyMove(const char **level, MovingEntity *blinky, const MovingEntity *pacman);

// Trouve la prochaine intersection que Pacman rencontrera en avançant
Coord getPacmanNextIntersection(const char **level, MovingEntity pacman);

// Gère la consommation d'une pastille / super pacgum et la mise à jour des stats
void eat(char **level, MovingEntity *pacman, MovingEntity ghosts[], int ghostCount, GameStats *stats);

// Boucle principale du jeu (point d'entrée pour l'exécution non-test)
int gameLoop(void);

// Gère la collision entre Pacman et un fantôme ; retourne true si la partie se termine
bool handleGhostCollision(MovingEntity *pacman, MovingEntity *ghost, GameStats *stats);

// Réinitialise les positions après la perte d'une vie
void resetPositionsAfterLifeLoss(char **level, MovingEntity *pacman, MovingEntity ghosts[], int ghostCount);

// Affiche l'animation de mort (court flash, mise en évidence)
void showDeathAnimation(RendererParameters *params, const MovingEntity *pacman, char **level, const Textures *textures, MovingEntity ghosts[], int ghostCount);

// Dessine le score et les vies sur l'interface
void drawScore(GameStats *stats, RendererParameters *params, SDL_Texture *pacmanTexture);

// Affiche les résultats finaux (victoire ou défaite)
void showFinalResults(int won, GameStats *stats, RendererParameters *params, const Textures *textures);

// Dessine un texte simple centré pour l'écran de pause
void drawSimplePauseText(const char *text, int x, int y, RendererParameters *params);

// Vérifie si le joueur gagne une vie supplémentaire selon le score
void checkForExtraLife(GameStats *stats);
#endif //MAIN_H
