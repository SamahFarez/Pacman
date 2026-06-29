#include "framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#ifdef TEST_MODE
// Inclure les en-têtes de test
#include "../tests/framework_mock.c" // Vous devrez peut-être créer ce fichier
#endif

/* ---------------------------------------------------------------------------
 * framework.c
 *
 * Description (FR):
 *   Couche d'abstraction pour SDL2 : initialisation du rendu, chargement
 *   de textures, fonctions d'affichage (sprites, niveaux, écrans finaux), et
 *   utilitaires graphiques comme le dessin de texte/numéros simples.
 *
 * Contrat (inputs/outputs) :
 *   - Entrées : paramètres de rendu (`RendererParameters`), structures `Textures`,
 *     chemins vers les ressources images.
 *   - Sortie : rendu sur `SDL_Renderer` géré via `params`.
 *
 * Points importants :
 *   - `init()` crée la fenêtre et le renderer, charge toutes les textures
 *     nécessaires et calcule la taille des cellules.
 *   - `getInput()` renvoie des touches SDL (ou 0 si rien) et gère SDL_QUIT.
 *   - `drawSimpleText()` et `drawSimpleNumber()` sont des routines simples de
 *     rendu de texte bitmap (scaling supporté) utilisées pour l'UI minimaliste.
 *
 * Nouvelles fonctionnalités ajoutées (non demandées explicitement dans la doc):
 *   - Support d'un paramètre `scale` dans `drawSimpleText()` pour agrandir
 *     proprement les caractères (utile pour les écrans finaux et le HUD).
 *   - Améliorations visuelles : écran de pause semi-transparent, centrage des
 *     textures de victoire/défaite.
 *
 * Remarques :
 *   - Le code reste minimaliste : pas d'utilisation de SDL_ttf pour garder la
 *     dépendance légère (texte rendu pixel par pixel).
 * ------------------------------------------------------------------------- */

/* Séparateur de chemin selon la plateforme */
const char *pathSeparator =
#ifdef _WIN32
    "\\";
#else
    "/";
#endif

#define ZOOM 1.5
#define GRID_CELL_SIZE 14

/* Initialise SDL, la fenêtre, le renderer et charge les textures.
 * - params : structure renseignée avec window/renderer/timing
 * - textures : textures chargées pour les sprites
 * - width/height : dimensions de la fenêtre (-1,-1 => plein écran)
 * - fps : images par seconde cible
 */
void init(RendererParameters *params, Textures *textures, int width, int height, const int fps)
{
    srand(time(NULL));

    /* Initialisation SDL2 */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Framework -> SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    Uint32 windowFlags = SDL_WINDOW_SHOWN;

    if (width == -1 && height == -1)
    {
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        width = DM.w;
        height = DM.h;
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
    else
    {
        params->width = width;
        params->height = height;
    }

    /* Fenêtre */
    params->window = SDL_CreateWindow(
        "Pacman",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        windowFlags);

    if (!params->window)
    {
        printf("Framework -> SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    /* Renderer (rendu) */
    params->renderer = SDL_CreateRenderer(
        params->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!params->renderer)
    {
        printf("Framework -> SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(params->window);
        SDL_Quit();
        exit(1);
    }

    SDL_SetRenderDrawColor(params->renderer, 0, 0, 0, 255);
    SDL_RenderClear(params->renderer);
    SDL_RenderPresent(params->renderer);

    /* Chargement des textures */
    textures->texturePacman = getTexture("pacman.bmp", params);
    textures->textureWall = getTexture("wall.bmp", params);
    textures->textureDot = getTexture("dot.bmp", params);
    textures->textureSuperPacgum = getTexture("super_pacgum.bmp", params);
    textures->textureBlinky = getTexture("blinky.bmp", params);
    textures->textureBlinkyL = getTexture("blinkyL.bmp", params);
    textures->textureBlinkyU = getTexture("blinkyU.bmp", params);
    textures->textureBlinkyD = getTexture("blinkyD.bmp", params);
    textures->texturePinky = getTexture("pinky.bmp", params);
    textures->texturePinkyL = getTexture("pinkyL.bmp", params);
    textures->texturePinkyU = getTexture("pinkyU.bmp", params);
    textures->texturePinkyD = getTexture("pinkyD.bmp", params);
    textures->textureInky = getTexture("inky.bmp", params);
    textures->textureInkyL = getTexture("inkyL.bmp", params);
    textures->textureInkyU = getTexture("inkyU.bmp", params);
    textures->textureInkyD = getTexture("inkyD.bmp", params);
    textures->textureClyde = getTexture("clyde.bmp", params);
    textures->textureClydeL = getTexture("clydeL.bmp", params);
    textures->textureClydeU = getTexture("clydeU.bmp", params);
    textures->textureClydeD = getTexture("clydeD.bmp", params);
    textures->textureBLue = getTexture("blue.bmp", params);
    textures->textureGameOver = getTexture("gameover.bmp", params);
    textures->texturePause = getTexture("pause.bmp", params);
    textures->textureWon = getTexture("won.bmp", params);

    textures->cellSize = ZOOM * GRID_CELL_SIZE;

    params->ticks_for_next_frame = 1000u / (Uint32)fps;
    params->lastTimeScreenUpdate = SDL_GetTicks();
}

/* Initialise le renderer et les textures avec la taille du plateau (fenêtré).
 * Utilise `init()` en fournissant les dimensions calculées.
 */
void initWindowed(RendererParameters *params, Textures *textures)
{
    params->width = (int)(WIDTH * ZOOM * GRID_CELL_SIZE);
    params->height = (int)(HEIGHT * ZOOM * GRID_CELL_SIZE);
    init(params, textures, params->width, params->height, FPS);
}

/* Récupère un événement SDL non bloquant :
 * - retourne SDL_QUIT si l'utilisateur ferme la fenêtre
 * - retourne le code de la touche en cas d'appui
 * - retourne 0 si aucun événement clavier n'est présent
 */
int getInput(void)
{
    SDL_Event e;
    if (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return SDL_QUIT;
        if (e.type == SDL_KEYDOWN)
            return e.key.keysym.sym;
    }
    return 0;
}

/* Attente / synchronisation pour respecter la fréquence FPS cible, puis
 * présentation du rendu courant et préparation du frame suivant.
 */
void update(RendererParameters *params)
{
    Uint32 now = SDL_GetTicks();
    while (now - params->lastTimeScreenUpdate < params->ticks_for_next_frame)
    {
        SDL_Delay(1);
        now = SDL_GetTicks();
    }

    SDL_RenderPresent(params->renderer);
    SDL_SetRenderDrawColor(params->renderer, 0, 0, 0, 255);
    SDL_RenderClear(params->renderer);

    params->lastTimeScreenUpdate = SDL_GetTicks();
}

/* Dessine une texture sur la grille : convertit les coordonnées grille ->
 * pixels et appelle SDL_RenderCopyEx avec l'angle donné.
 */
void drawSprite(SDL_Texture *texture, int x, int y, float angle, const RendererParameters *params)
{
    SDL_Rect dest = {
        x * GRID_CELL_SIZE * ZOOM,
        y * GRID_CELL_SIZE * ZOOM,
        GRID_CELL_SIZE * ZOOM,
        GRID_CELL_SIZE * ZOOM};

    SDL_RenderCopyEx(
        params->renderer,
        texture,
        NULL,
        &dest,
        angle,
        NULL,
        SDL_FLIP_NONE);
}

// In src/framework.c (not the test file)

/* Parcourt le tableau `level` et dessine les éléments statiques (murs,
 * pac-gommes, super pac-gommes). Les entités mobiles (Pacman, fantômes)
 * sont dessinées séparément par le code de `main.c`.
 */
void drawLevel(char **level, const RendererParameters *params, const Textures *textures)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            char cell = level[y][x];

            switch (cell)
            {
            case 'H': // Wall
                drawSprite(textures->textureWall, x, y, 0, params);
                break;
            case '.': // Dot
                drawSprite(textures->textureDot, x, y, 0, params);
                break;
            case 'G': // Super Pacgum
                drawSprite(textures->textureSuperPacgum, x, y, 0, params);
                break;
            // Note: We don't draw ghosts or Pacman here - they're drawn separately
            // 'P', 'I', 'C', 'B', 'O' are handled elsewhere
            default:
                // Empty space - do nothing
                break;
            }
        }
    }
}

/* Wrapper public pour dessiner un sprite sur la grille.
 * Utilisé par le reste du jeu pour dessiner Pacman/fantômes.
 */
void drawSpriteOnGrid(SDL_Texture *texture, int x, int y, float angle, const RendererParameters *params)
{
    drawSprite(texture, x, y, angle, params);
}

/* Charge une image BMP et crée une SDL_Texture. Chemin relatif attendu
 * depuis le répertoire de build vers resources/images.
 */
SDL_Texture *getTexture(char *imageName, const RendererParameters *params)
{
    char imagePath[256];
    // Depuis le répertoire de build (pacman/src/build/) vers resources (pacman/resources/images/)
    snprintf(imagePath, sizeof(imagePath),
             "../../resources/images/%s", imageName);

    SDL_Surface *sprite = SDL_LoadBMP(imagePath);
    if (!sprite)
    {
        printf("SDL_LoadBMP failed for '%s': %s\n", imagePath, SDL_GetError());
        exit(1);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(params->renderer, sprite);
    SDL_FreeSurface(sprite);

    if (!texture)
    {
        printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        exit(1);
    }

    return texture;
}

/* Retourne un entier pseudo-aléatoire (wrapper sur rand()). */
int getRandomNumber(void)
{
    return rand();
}

/* Dessine l'écran de victoire : texture centrée. */
void drawWonScreen(const RendererParameters *params, const Textures *textures)
{
    SDL_SetRenderDrawColor(params->renderer, 0, 0, 0, 255); // Fond (noir)
    SDL_RenderClear(params->renderer);

    // Dessiner la texture de victoire centrée à l'écran
    if (textures->textureWon)
    {
        // Récupérer les dimensions de la texture
        int texWidth, texHeight;
        SDL_QueryTexture(textures->textureWon, NULL, NULL, &texWidth, &texHeight);

        // Center the texture
        SDL_Rect destRect = {
            (params->width - texWidth) / 2,
            (params->height - texHeight) / 2,
            texWidth,
            texHeight};

        SDL_RenderCopy(params->renderer, textures->textureWon, NULL, &destRect);
    }
}

/* Dessine l'écran de défaite (game over) avec la texture centrée. */
void drawGameOverScreen(const RendererParameters *params, const Textures *textures)
{
    // Effacer l'écran en noir
    SDL_SetRenderDrawColor(params->renderer, 0, 0, 0, 255);
    SDL_RenderClear(params->renderer);

    // Dessiner la texture 'game over' centrée à l'écran
    if (textures->textureGameOver)
    {
        // Récupérer les dimensions de la texture
        int texWidth, texHeight;
        SDL_QueryTexture(textures->textureGameOver, NULL, NULL, &texWidth, &texHeight);

        // Center the texture
        SDL_Rect destRect = {
            (params->width - texWidth) / 2,
            (params->height - texHeight) / 2,
            texWidth,
            texHeight};

        SDL_RenderCopy(params->renderer, textures->textureGameOver, NULL, &destRect);
    }
}

/* Dessine l'écran de pause : overlay semi-transparent + texture centrée
 * si disponible, sinon un rectangle clignotant.
 */
void drawPauseScreen(const RendererParameters *params, const Textures *textures)
{
    // Draw semi-transparent black overlay
    SDL_SetRenderDrawColor(params->renderer, 0, 0, 0, 180); // Semi-transparent black
    SDL_RenderFillRect(params->renderer, NULL);             // Remplir tout l'écran

    // Dessiner la texture de pause centrée
    if (textures->texturePause)
    {
        // Récupérer les dimensions de la texture
        int texWidth, texHeight;
        SDL_QueryTexture(textures->texturePause, NULL, NULL, &texWidth, &texHeight);

        // Centrer la texture
        SDL_Rect destRect = {
            (params->width - texWidth) / 2,
            (params->height - texHeight) / 2,
            texWidth,
            texHeight};

        SDL_RenderCopy(params->renderer, textures->texturePause, NULL, &destRect);
    }
    else
    {
        // Repli : dessiner un texte simple clignotant
        static Uint32 lastBlinkTime = 0;
        static int blink = 1;

        if (SDL_GetTicks() - lastBlinkTime > 500)
        {
            blink = !blink;
            lastBlinkTime = SDL_GetTicks();
        }

        if (blink)
        {
            SDL_SetRenderDrawColor(params->renderer, 255, 255, 255, 255);
            SDL_Rect pausedRect = {params->width / 2 - 60, params->height / 2 - 30, 120, 20};
            SDL_RenderFillRect(params->renderer, &pausedRect);
        }
    }
}

/* Dessine du texte simple pixel-par-pixel. Le paramètre `scale` agrandit
 * les caractères (utile pour titres / écrans finaux).
 */
void drawSimpleText(const char *text, int x, int y, const RendererParameters *params, int scale)
{
    if (scale < 1)
        scale = 1; // Minimum scale of 1

    SDL_SetRenderDrawColor(params->renderer, 255, 255, 255, 255); // White

    for (int i = 0; text[i] != '\0'; i++)
    {
        char c = text[i];
        int charX = x + i * (7 * scale); // 7 pixels par caractère (espacement inclus)

        // Dessiner chaque caractère selon sa forme, avec mise à l'échelle
        switch (c)
        {
        case 'F':
            // Caractère 'F' - mise à l'échelle
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }
                if (py < scale || (py >= 3 * scale && py < 4 * scale))
                {
                    for (int hx = 0; hx < 3 * scale; hx++)
                    {
                        for (int px = 0; px < scale; px++)
                        {
                            SDL_RenderDrawPoint(params->renderer, charX + scale + hx + px, y + py);
                        }
                    }
                }
            }
            break;
        case 'I':
            // Caractère 'I' - mise à l'échelle
            for (int px = 0; px < 4 * scale; px++)
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    for (int py = 0; py < scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + (6 * scale) + py);
                    }
                }
            }
            for (int py = scale; py < 6 * scale; py++)
            {
                for (int px = 0; px < 2 * scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + scale + px, y + py);
                }
            }
            break;
        case 'N':
            // Caractère 'N' - mise à l'échelle
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                    SDL_RenderDrawPoint(params->renderer, charX + (3 * scale) + px, y + py);
                }
                // Diagonale
                int diagPos = (py * 3 * scale) / (7 * scale);
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + diagPos + px, y + py);
                }
            }
            break;
        case 'A':
            // Caractère 'A' - mise à l'échelle
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                    SDL_RenderDrawPoint(params->renderer, charX + (3 * scale) + px, y + py);
                }
                if (py < scale || (py >= 3 * scale && py < 4 * scale))
                {
                    for (int hx = 0; hx < 2 * scale; hx++)
                    {
                        for (int px = 0; px < scale; px++)
                        {
                            SDL_RenderDrawPoint(params->renderer, charX + scale + hx + px, y + py);
                        }
                    }
                }
            }
            break;
        case 'L':
            // Caractère 'L' - mise à l'échelle
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }
                if (py >= 6 * scale)
                {
                    for (int hx = 0; hx < 3 * scale; hx++)
                    {
                        for (int px = 0; px < scale; px++)
                        {
                            SDL_RenderDrawPoint(params->renderer, charX + scale + hx + px, y + py);
                        }
                    }
                }
            }
            break;
        case 'S':
            // Caractère 'S' - mise à l'échelle
            for (int px = 0; px < 4 * scale; px++)
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    // Partie horizontale supérieure
                    for (int py = 0; py < scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                    // Partie horizontale centrale
                    for (int py = 3 * scale; py < 4 * scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                    // Partie horizontale inférieure
                    for (int py = 6 * scale; py < 7 * scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                }
            }
            // Courbures côté gauche
            for (int px = 0; px < scale; px++)
            {
                for (int py = scale; py < 3 * scale; py++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }
            }
            // Courbures côté droit
            for (int px = 3 * scale; px < 4 * scale; px++)
            {
                for (int py = 4 * scale; py < 6 * scale; py++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }
            }
            break;
        case 'C':
            // Caractère 'C' - mise à l'échelle
            for (int px = 0; px < 4 * scale; px++)
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    // Haut
                    for (int py = 0; py < scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                    // Bas
                    for (int py = 6 * scale; py < 7 * scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                }
            }
            for (int py = scale; py < 6 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }
            }
            break;
        case 'O':
            // Caractère 'O' - mise à l'échelle
            for (int px = 0; px < 4 * scale; px++)
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    // Haut
                    for (int py = 0; py < scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                    // Bas
                    for (int py = 6 * scale; py < 7 * scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                }
            }
            for (int py = scale; py < 6 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                    SDL_RenderDrawPoint(params->renderer, charX + (3 * scale) + px, y + py);
                }
            }
            break;
        case 'R':
            for (int py = 0; py < 7 * scale; py++)
            {
                // Tige gauche
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }

                // Côté droit du renflement (moitié supérieure)
                if (py < 4 * scale)
                {
                    for (int px = 0; px < scale; px++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + 3 * scale + px, y + py);
                    }
                }

                // Barres horizontales haut + milieu (CENTRÉES)
                if (py < scale || (py >= 3 * scale && py < 4 * scale))
                {
                    for (int px = 0; px < 2 * scale; px++)
                    {
                        SDL_RenderDrawPoint(
                            params->renderer,
                            charX + scale + px,
                            y + py);
                    }
                }

                // Jambe diagonale
                if (py >= 4 * scale)
                {
                    int offset = (py - 4 * scale) / scale;
                    for (int px = 0; px < scale; px++)
                    {
                        SDL_RenderDrawPoint(
                            params->renderer,
                            charX + scale + offset * scale + px,
                            y + py);
                    }
                }
            }
            break;

        case 'E':
            // Caractère 'E' - mise à l'échelle
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }
                if (py < scale || (py >= 3 * scale && py < 4 * scale) || py >= 6 * scale)
                {
                    for (int hx = 0; hx < 3 * scale; hx++)
                    {
                        for (int px = 0; px < scale; px++)
                        {
                            SDL_RenderDrawPoint(params->renderer, charX + scale + hx + px, y + py);
                        }
                    }
                }
            }
            break;
        case 'Y':
            // Caractère 'Y' - mise à l'échelle
            for (int py = 0; py < 3 * scale; py++)
            {
                int offset = py / scale;
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + (offset * scale) + px, y + py);
                    SDL_RenderDrawPoint(params->renderer, charX + (3 * scale) - (offset * scale) + px, y + py);
                }
            }
            for (int py = 3 * scale; py < 7 * scale; py++)
            {
                for (int px = 0; px < 2 * scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + scale + px, y + py);
                }
            }
            break;
        case 'U':
            // Caractère 'U' - mise à l'échelle
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                    if (py >= 6 * scale)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + scale + px, y + py);
                        SDL_RenderDrawPoint(params->renderer, charX + (2 * scale) + px, y + py);
                    }
                    SDL_RenderDrawPoint(params->renderer, charX + (3 * scale) + px, y + py);
                }
            }
            break;
        case 'W':
            // Caractère 'W' - mise à l'échelle
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                    SDL_RenderDrawPoint(params->renderer, charX + (4 * scale) + px, y + py);
                }
                if (py >= 4 * scale)
                {
                    int offset = (py - 4 * scale) / (3 * scale) * 2 * scale;
                    for (int px = 0; px < scale; px++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + scale + offset + px, y + py);
                        SDL_RenderDrawPoint(params->renderer, charX + (3 * scale) - offset + px, y + py);
                    }
                }
                else
                {
                    for (int px = 0; px < scale; px++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + (2 * scale) + px, y + py);
                    }
                }
            }
            break;
        case 'T':
            // Caractère 'T' - mise à l'échelle
            for (int px = 0; px < 4 * scale; px++)
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    for (int py = 0; py < scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px + sx, y + py);
                    }
                }
            }
            for (int py = scale; py < 7 * scale; py++)
            {
                for (int px = 0; px < 2 * scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + scale + px, y + py);
                }
            }
            break;
        case 'P':
            for (int py = 0; py < 7 * scale; py++)
            {
                // Tige gauche
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                }

                // Côté droit du renflement (seulement moitié supérieure)
                if (py < 4 * scale)
                {
                    for (int px = 0; px < scale; px++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + 3 * scale + px, y + py);
                    }
                }

                // Barres horizontales haut et milieu (CENTRÉES)
                if (py < scale || (py >= 3 * scale && py < 4 * scale))
                {
                    for (int px = 0; px < 2 * scale; px++)
                    {
                        SDL_RenderDrawPoint(
                            params->renderer,
                            charX + scale + px,
                            y + py);
                    }
                }
            }
            break;

        case 'K':
            // Tige verticale gauche
            for (int py = 0; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(
                        params->renderer,
                        charX + px,
                        y + py);
                }
            }

            // Diagonales supérieure et inférieure
            for (int py = 0; py < 3 * scale; py++)
            {
                int step = py / scale;

                for (int px = 0; px < scale; px++)
                {
                    // Diagonale supérieure (du milieu vers le haut-droite)
                    SDL_RenderDrawPoint(
                        params->renderer,
                        charX + scale + step * scale + px,
                        y + (3 * scale) - py);

                    // Diagonale inférieure (du milieu vers le bas-droite)
                    SDL_RenderDrawPoint(
                        params->renderer,
                        charX + scale + step * scale + px,
                        y + (3 * scale) + py);
                }
            }
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // Chiffres
            drawSimpleNumber(c, charX, y, params, scale);
            break;
        case ':':
            // Deux-points pour le score - dessiner deux points
            for (int dot = 0; dot < 2; dot++)
            {
                int dotY = y + (2 * scale) + (dot * 3 * scale);
                for (int px = 0; px < scale; px++)
                {
                    for (int py = 0; py < scale; py++)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + (scale * 2) + px, dotY + py);
                    }
                }
            }
            break;
        case '!':
            // Point d'exclamation
            for (int py = 0; py < 6 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + (2 * scale) + px, y + py);
                }
            }
            for (int py = 6 * scale; py < 7 * scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    SDL_RenderDrawPoint(params->renderer, charX + (2 * scale) + px, y + py);
                }
            }
            break;
        case ' ':
            // Espace - ne rien dessiner
            break;
        default:
            // Pour les caractères inconnus, dessiner un encadré
            for (int px = 0; px < 4 * scale; px++)
            {
                for (int py = 0; py < 7 * scale; py++)
                {
                    if (px < scale || px >= 3 * scale || py < scale || py >= 6 * scale)
                    {
                        SDL_RenderDrawPoint(params->renderer, charX + px, y + py);
                    }
                }
            }
            break;
        }
    }
}

/* Petit utilitaire interne pour dessiner un chiffre (0-9) avec le même
 * système de pixel-art que drawSimpleText. Exporté via le header pour tests.
 */
void drawSimpleNumber(char num, int x, int y, const RendererParameters *params, int scale)
{
    switch (num)
    {
    case '0':
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < 7 * scale; py++)
                if (px < scale || px >= 3 * scale || py < scale || py >= 6 * scale)
                    SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    case '1':
        // Barre verticale
        for (int px = 0; px < scale; px++)
            for (int py = 0; py < 7 * scale; py++)
                SDL_RenderDrawPoint(
                    params->renderer,
                    x + (2 * scale) + px,
                    y + py);
        break;

    case '2':
        // Haut
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Partie supérieure droite
        for (int px = 3 * scale; px < 4 * scale; px++)
            for (int py = scale; py < 3 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Milieu
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 3 * scale; py < 4 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Bas gauche
        for (int px = 0; px < scale; px++)
            for (int py = 4 * scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Bas
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 6 * scale; py < 7 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    case '3':
        // Haut, milieu, bas
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
            {
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (3 * scale) + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (6 * scale) + py);
            }

        // Côté droit
        for (int px = 3 * scale; px < 4 * scale; px++)
            for (int py = scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    case '4':
        // Tige verticale gauche
        for (int px = 0; px < scale; px++)
            for (int py = 0; py < 4 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Middle bar
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 3 * scale; py < 4 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Tige verticale droite
        for (int px = 3 * scale; px < 4 * scale; px++)
            for (int py = 0; py < 7 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    case '5':
        // Haut, milieu, bas
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
            {
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (3 * scale) + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (6 * scale) + py);
            }

        // Haut gauche
        for (int px = 0; px < scale; px++)
            for (int py = scale; py < 3 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Bas droit
        for (int px = 3 * scale; px < 4 * scale; px++)
            for (int py = 4 * scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    case '6':
        // Haut
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Milieu et bas
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
            {
                SDL_RenderDrawPoint(params->renderer, x + px, y + (3 * scale) + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (6 * scale) + py);
            }

        // Côté gauche
        for (int px = 0; px < scale; px++)
            for (int py = scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Bas droit
        for (int px = 3 * scale; px < 4 * scale; px++)
            for (int py = 4 * scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    case '7':
        // Haut
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

    // Diagonale
        for (int py = 0; py < 6 * scale; py++)
        {
            int step = py / scale;
            for (int px = 0; px < scale; px++)
                SDL_RenderDrawPoint(
                    params->renderer,
                    x + (3 * scale) - step * scale + px,
                    y + scale + py);
        }
        break;

    case '8':
        // Haut, milieu, bas
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
            {
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (3 * scale) + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (6 * scale) + py);
            }

        // Côtés gauche et droit
        for (int px = 0; px < scale; px++)
            for (int py = scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        for (int px = 3 * scale; px < 4 * scale; px++)
            for (int py = scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    case '9':
        // Haut, milieu et bas
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < scale; py++)
            {
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (3 * scale) + py);
                SDL_RenderDrawPoint(params->renderer, x + px, y + (6 * scale) + py);
            }

        // Haut gauche
        for (int px = 0; px < scale; px++)
            for (int py = scale; py < 3 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);

        // Côté droit
        for (int px = 3 * scale; px < 4 * scale; px++)
            for (int py = scale; py < 6 * scale; py++)
                SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;

    default:
        // Valeur par défaut = 0
        for (int px = 0; px < 4 * scale; px++)
            for (int py = 0; py < 7 * scale; py++)
                if (px < scale || px >= 3 * scale || py < scale || py >= 6 * scale)
                    SDL_RenderDrawPoint(params->renderer, x + px, y + py);
        break;
    }
}

/* Nettoie les ressources allouées et termine l'utilisation de SDL.
 *
 * Paramètres :
 *  - params : pointeur vers la structure RendererParameters. Si non-NULL,
 *    les objets SDL associés (renderer, window) seront détruits.
 *  - level : tableau de chaînes représentant le niveau (hauteur = HEIGHT).
 *    Si non-NULL, chaque ligne est libérée puis le tableau lui-même.
 *
 * Effets de bord : détruit le renderer/window si présents et appelle
 * SDL_Quit() pour fermer la sous-couche SDL. Cette fonction est sûre à
 * appeler avec des pointeurs NULL (aucune action dans ce cas).
 */
void cleanUp(const RendererParameters *params, char **level)
{
    if (level)
    {
        for (int i = 0; i < HEIGHT; i++)
            free(level[i]);
        free(level);
    }

    if (params)
    {
        if (params->renderer)
            SDL_DestroyRenderer(params->renderer);
        if (params->window)
            SDL_DestroyWindow(params->window);
    }

    SDL_Quit();
}
