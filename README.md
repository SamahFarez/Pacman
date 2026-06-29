# Pacman — Mini-projet (Rapport et mode d'emploi)

Ce dépôt contient une version pédagogique de Pacman développée dans le cadre du cours. Ce README décrit :

- Ce qu'est le jeu et son objectif pédagogique.
- Les fonctionnalités demandées par le sujet (fiches dans `doc/consignes`) et où elles sont implémentées.0
- Les nouvelles fonctionnalités et ajustements ajoutés pour rendre le code testable et cohérent.
- Comment compiler, lancer le jeu et exécuter les tests unitaires.
- Notes pratiques et suggestions d'amélioration.

## 1) Qu'est-ce que ce jeu ?

Pacman est un jeu d'arcade classique. Le joueur contrôle Pacman sur une grille (28x31) qui contient :

- Des murs (H),
- Des pacgums ('.'),
- Des super pacgums (gros points),
- Des fantômes (Blinky, Pinky, Inky, Clyde),
- Pacman lui-même (O dans les niveaux de départ).

But du jeu : manger toutes les pacgums sans perdre toutes ses vies. Pacman se déplace case par case sur la grille ; les fantômes suivent des stratégies différentes.

Ce projet est une mise en œuvre pédagogique fondée sur un framework d'affichage (SDL2). Le code est structuré pour faciliter les tests unitaires (mode `PACMAN_TESTS`).

---

## 2) Fonctionnalités demandées (récapitulatif des consignes)

Les consignes se trouvent dans `doc/consignes/` et décrivent étape par étape le travail attendu. Voici les points principaux et où les trouver dans le code :

1. Prendre en main le framework d'affichage (doc `1_Prendre en main le framework d'affichage.adoc`)
   - Fichiers : `src/framework.h` et `src/framework.c`.
   - Contenu : initialisation SDL, chargement de textures, fonctions `drawLevel`, `drawSpriteOnGrid`, `drawSimpleText`, etc. Le framework masque la complexité SDL et fournit des utilitaires prêts à l'emploi.

2. Déplacements sur la grille (doc `2_Deplacements sur la grille.adoc`)
   - Fichiers : `src/main.c`, `src/main.h`.
   - Mise en œuvre : chargement du niveau (`loadFirstLevel`), dessin de la grille (`drawLevel`) et gestion du déplacement case par case. Le projet gère les collisions avec les murs et le téléport horizontal (wrap).

3. La boucle de jeu (doc `3_La boucle de jeu.adoc`)
   - Fichier : `src/main.c`.
   - Fonction : `gameLoop()` implémente la boucle input → logique → draw, avec update en fin de cycle. Les fonctions `eat()` et `win()` gèrent respectivement la consommation de pacgums et la condition de victoire.

4. Déplacement de Pacman (doc `4_Deplacement de pacman partie 2.adoc`)
   - Conservation de la dernière commande, mémorisation de la prochaine direction si elle n'est pas immédiatement exécutable, et continuation du mouvement tant qu'aucune nouvelle commande valide n'est reçue. Implémenté dans la boucle principale et gestion des entrées.

5. Comportements des fantômes (docs `5_Clyde.adoc`, `6_Pinky.adoc`, `7_Inky.adoc`, `8_Blinky.adoc`)
   - `clydeMove`, `pinkyMove`, `inkyMove`, `blinkyMove` dans `src/main.c`.
   - Règles :
     - Les fantômes ne font pas de demi-tour sauf en impasse.
     - Clyde : déplacement principalement aléatoire.
     - Pinky : poursuit Pacman si ligne de vue, sinon se comporte comme Clyde.
     - Inky : essaye de réduire la plus grande composante de distance (horiz/vert) vers Pacman, avec un peu d'aléa (`INKY_RANDOMNESS`).
     - Blinky : vise la prochaine intersection que Pacman atteindra.

6. Super Pacgum (doc `9_Super Pacgum.adoc`)
   - Comportement : les fantômes deviennent FRIGHTENED (bleus) et font demi-tour ; Pacman peut les manger (ils retournent à leur position de départ en état EATEN). Géré dans `eat()` et la logique de `ghostMove`.

7. Divers
   - Texte/affichage des textures : `10_Textures.adoc` explique la gestion des sprites dans `resources/images` et leur usage via `Textures`.

---

## 3) Fonctionnalités de jeu ajoutées et améliorations (détail)

La partie suivante décrit les vraies fonctionnalités de jeu ajoutées ou précisées dans le code — celles qui affectent l'expérience du joueur (écrans, score, vies, animations, etc.). Ces éléments sont implémentés dans `src/main.c` et utilisent les utilitaires de rendu du framework (`src/framework.c`).

1) Écran de victoire (win)
   - Comportement : lorsque toutes les pacgums du niveau sont mangées, le jeu affiche un écran de victoire qui présente le score final et des statistiques (pastilles mangées, fantômes mangés, vies restantes).
   - Emplacement : fonction `showFinalResults()` et code lié dans `gameLoop()`.
   - Détail : le texte est centré dans le quart inférieur de l'écran avec une mise en évidence du score (scale 2-3) pour la lisibilité.

2) Écran de défaite (game over)
   - Comportement : quand le joueur perd toutes ses vies, l'écran de "Game Over" s'affiche avec le score final et la possibilité de redémarrer.
   - Emplacement : également géré par `showFinalResults()` et la logique autour de `gameLoop()`.

3) Pause et redémarrage
   - Pause : le jeu peut être mis en pause (touche dédiée) ; la boucle principale conserve l'état et affiche un message de pause discret (texte centré). Le rendu est gelé tant que la pause est active.
   - Redémarrage : après une défaite ou depuis l'écran de victoire, le joueur peut redémarrer une nouvelle partie (réinitialisation du niveau, des positions et des statistiques via `resetPositionsAfterLifeLoss()` et initialisation des `GameStats`).

4) Animation de mort (death animation)
   - Comportement : lorsqu'une vie est perdue, une courte animation montre Pacman se faire manger (ou clignoter) et l'écran effectue un flash rouge court pour signaler l'événement au joueur.
   - Emplacement : fonction `showDeathAnimation()` et séquence dans la boucle principale.
   - Détail : l'animation est volontairement courte en mode test mais complète en mode interactif (plusieurs étapes de clignotement et pause entre chaque frame).

5) Système de vies (3 vies)
   - Comportement : le joueur démarre avec `STARTING_LIVES` (valeur définie dans `src/main.h`, par défaut 3). À chaque collision fatale, `stats.lives` est décrémenté. Si `stats.lives` atteint 0, la partie se termine.
   - Emplacement : gestion dans `handleGhostCollision()` et `gameLoop()`.

6) Système de score
   - Points de base :
     - Pacgum simple : `SCORE_DOT` points (défini dans `src/main.h`).
     - Super pacgum : `SCORE_SUPER_PACGUM` points et activation du mode FRIGHTENED.
   - Affichage : le score s'affiche en haut à gauche via `drawScore()` et les petites icônes Pacman indiquent le nombre de vies restantes.

7) Multiplicateur pour fantômes mangés en chaîne
   - Comportement : quand Pacman mange un fantôme pendant le mode FRIGHTENED, le score attribué est multiplié par `ghostMultiplier` et `ghostMultiplier` est ensuite doublé (ou multiplié par `SCORE_GHOST_MULTIPLIER`) pour le fantôme suivant mangé avant expiration du mode frightened.
   - Emplacement : gestion dans `eat()` et `handleGhostCollision()` ; les constantes `SCORE_GHOST_BASE` et `SCORE_GHOST_MULTIPLIER` sont définies dans `src/main.h`.

8) Affichage HUD simple
   - Le HUD affiche le score, le texte "LIVES:" et des petites icônes Pacman pour chacune des vies restantes. Ce rendu est assuré par `drawScore()` et les utilitaires de `framework.c`.

Notes d'implémentation et de configuration
   - Les constantes (points, durée frightened, vies de départ, etc.) se trouvent dans `src/main.h` pour être faciles à modifier.
   - Les animations et effets visuels utilisent les fonctions du framework ; si vous voulez prolonger une animation (ex : faire une animation de mort plus longue), adaptez `showDeathAnimation()` en conséquence.

---

## 4) Détails par fonctionnalité (explication technique)

Je décris ici, pour chaque fonctionnalité principale, le contrat (entrée/sortie) et les points d'attention pour les tests et l'évolution.

A) gameLoop()
- Entrées : aucune (utilise des fonctions du framework pour les entrées, textures et le niveau).
- Effets : boucle principale qui traite entrées -> logique des entités -> collisions -> rendu.
- Retour : un entier ; >0 indique victoire, <0 indique défaite (avec -score-1 en cas de défaite), 0 indique sortie normale.
- Points d'attention : pour les tests nous exposons `PACMAN_TESTS` qui raccourcit les délais et fait retourner la boucle plus tôt.

B) eat(level, pacman, ghosts, ...)
- Rôle : vérifier si la case sur laquelle Pacman arrive contient une pacgum ou une super pacgum.
- Effets : mise à jour du niveau (`level[y][x] = ' '`), incrément du score, activation du mode FRIGHTENED pour les fantômes en cas de super pacgum.
- Détail : le multiplicateur de fantômes est incrémenté si Pacman mange des fantômes à la suite.

C) getPotentialDirections(level, entity, *nbDir)
- Rôle : retourner la liste dynamique des directions valides depuis la position courante d'une entité.
- Règles :
  - Une direction n'est valide que si la case cible n'est pas un mur.
  - On exclut la direction opposée (pas de demi-tour), sauf si on est en impasse.
  - Le wrap horizontal est pris en compte.

D) ghostMove / clydeMove / pinkyMove / inkyMove / blinkyMove
- Contrat : chque fonction prend le `level` et la structure `MovingEntity *` du fantôme (et parfois la position de Pacman) et modifie `ghost->dir` et/ou `ghost->pos`.
- Stratégies : voir section 2 (Clyde, Pinky, Inky, Blinky).
- Détails : si un fantôme est en état FRIGHTENED, il se déplace de manière aléatoire et revient à un état NORMAL après expiration du timer.

E) checkCollision / handleGhostCollision
- Rôle : détecter collisions directes et croisées, appliquer effets (perte de vie ou pacman mange fantôme selon l'état du fantôme).
- Spécificité tests : en mode `PACMAN_TESTS`, la détection/gestion est parfois plus stricte pour garantir un flux de test déterministe.

---

## 5) Comment compiler et exécuter

Prérequis :

- Système d'exploitation testé : Ubuntu Linux (tests et développement effectués sur une distri GNU/Linux). Le projet **n'a pas été testé sous Windows** — des ajustements pour l'environnement Windows peuvent être nécessaires.
- Outils de compilation : GCC (ou clang), CMake, make (ou équivalent build system), paquet "build-essential" sur Debian/Ubuntu.
- Bibliothèques runtime/de développement :
   - SDL2 (dev) : package `libsdl2-dev` sur Debian/Ubuntu.
   - cmocka (pour exécuter la suite de tests) : package `libcmocka-dev` sur Debian/Ubuntu.

Exemple d'installation des dépendances sur Ubuntu/Debian :

```bash
sudo apt update
sudo apt install -y build-essential cmake libsdl2-dev libcmocka-dev
```

Remarque pour macOS : installez les outils depuis Homebrew si besoin :

```bash
brew install sdl2 cmocka cmake
```

Remarque pour Windows : le projet n'a pas été testé sous Windows. Si vous voulez l'exécuter sous Windows, installez les équivalents (SDL2 dev) et adaptez la chaîne de build (MSYS2, MinGW, ou Visual Studio) ; signalez-moi si vous souhaitez que je fournisse des instructions Windows.

Note : assurez-vous que le dossier `resources/images/` contient les fichiers BMP nécessaires au rendu ; sinon le jeu peut démarrer sans visuels attendus.


Compilation et exécution normale (jeu interactif) via CMake :

Ouvrez un terminal dans le répertoire `src/` puis exécutez exactement ces commandes :

```bash
rm -rf build
mkdir build
cd build
cmake ..
make -j4
./pacman
```

Remarque : selon la configuration CMake du dépôt, le binaire peut être placé dans `build/`. Dans ce cas, après la compilation vous pouvez lancer `./build/pacman` depuis la racine du projet.

Exécution de la suite de tests unitaires fournie :

Méthode A —  exécuter les tests via commande GCC

Depuis la racine du projet, vous pouvez compiler et exécuter les tests avec la commande GCC suivante qui active le mode test (`-DPACMAN_TESTS`) et produit l'exécutable `pacman_tests` :

```bash
gcc -DPACMAN_TESTS -I/usr/include/SDL2 -I/usr/include -Isrc -Itests \
   tests/test.c \
   tests/framework_mock.c \
   tests/firstLevel_stub.c \
   src/main.c \
   -lSDL2 -lcmocka -lm -o pacman_tests 2>/dev/null && \
   ./pacman_tests
```

Remarque : la commande ci-dessus redirige les erreurs de liaison vers `/dev/null` pour produire une sortie plus propre; si vous préférez voir les erreurs, supprimez `2>/dev/null`.

Si vous préférez CMake pour lancer les tests, la cible `pacman_tests` existe aussi dans le CMakeLists; l'exemple GCC ci-dessus est fourni car il reflète précisément la façon dont les tests peuvent être invoqués rapidement sans générer un build CMake.

Méthode B — exécuter les tests via CMake (recommandé si vous voulez un build reproductible)

Si vous voulez construire et lancer les tests en utilisant CMake (méthode alternative à la commande GCC ci-dessus), exécutez depuis la racine du dépôt :

```bash
rm -rf build
mkdir build
cd build
cmake ..
cmake --build . -j4
./pacman_tests
```

Remarques :
- La cible `pacman_tests` est créée par le `CMakeLists.txt` racine et génère l'exécutable `pacman_tests` dans le répertoire `build/`.
- Vous pouvez aussi utiliser `ctest` depuis le répertoire `build` pour exécuter les tests si vous préférez :

```bash
cd build
ctest --output-on-failure
```

---

## 6) Où regarder dans le code (guide rapide)

- `src/main.c` : logique principale, boucle de jeu, déplacements, collisions, scoring, fonctions de fantômes.
- `src/main.h` : types partagés (`Coord`, `MovingEntity`, `GameStats`) et prototypes publics.
- `src/framework.c` / `src/framework.h` : initialisation SDL, chargement textures, fonctions de dessin (abstraction du rendu).
- `src/firstLevel.c` (ou `src/firstLevel.*`) : chargement du niveau initial (tableau 28x31) et ressource des images.
- `tests/` : tests unitaires (ne pas modifier si vous devez garder le correcteur d'origine)

Pour modifier ou étendre un comportement des fantômes, commencez par `ghostMove` et les fonctions spécifiques (`clydeMove`, `pinkyMove`, ...).

---

## 7) Conseils de debug et problèmes courants

- Warnings lors de la compilation : certains avertissements (variables non utilisées dans des builds tests ou initialisateurs manquants pour les structures temporaires) sont présents et inoffensifs ; je peux nettoyer les warnings si vous le souhaitez.
- Si l'exécutable graphique ne démarre pas : vérifiez SDL2 dev installé et les includes (`#include <SDL2/SDL.h>`), et que les fichiers BMP existent dans `resources/images/`.
- Si les tests échouent : ne modifiez pas les fichiers `tests/` — préférez ajuster le code productif pour respecter les spécifications testées.

---

## 8) Pistes d'amélioration (faciles à intégrer)

- Utiliser `SDL_ttf` pour avoir du texte vectoriel et multilingue propre.
- Déplacer la logique des fantômes dans des fichiers séparés pour clarifier la lisibilité et faciliter les tests unitaires ciblés.
- Ajouter des tests unitaires supplémentaires (par exemple tests pour `getPotentialDirections`, `hasLineOfSight`).
- Remplacer la génération pseudo-aléatoire par un générateur conditionnable (seed) pour tests plus déterministes.

---
