#ifndef __LABYRINTH_GAME__
#define __LABYRINTH_GAME__

#include "laby.h"

#define L game->lab
#define P game->player
#define E game->exit

enum command
{
  CMD_NOTHING = 0,
  CMD_NEW_GAME = 1000,
  CMD_MV_LEFT,
  CMD_MV_UP,
  CMD_MV_RIGHT,
  CMD_MV_DOWN,
  CMD_PAUSE,
  CMD_CONTINUE,
  CMD_EXIT,
};

enum game_state
{
  ST_MAIN_MENU,
  ST_GAME,
  ST_PAUSE,
  ST_WIN
};

struct render;
typedef struct render Render;

struct menu;
typedef struct menu Menu;

typedef struct
{
  unsigned char row;
  unsigned char col;
  /* The radius of visible distance including the y:x.
   * 1 means that only current y:x room is visible.
   * 0 turns off visibility check completely. */
  unsigned char visible_range;
} Player;

typedef struct
{
  /* Global configuration of the game */
  int seed;
  /* The count of rooms by vertical in the new laby */
  int laby_rows;
  /* The count of rooms by horizontal in the new laby */
  int laby_cols;

  Render *render;
  /* ------------------------------- */

  /* The FSM of the game  */
  enum game_state state;
  /* The current labyrinth */
  Laby lab;
  /* The current state of the player */
  Player player;
  /* Implementation of a menu depends on runtime.
   * The main logic of the game doesn't depend on a menu
   * implementation.*/
  Menu *menu;
} Game;

/**
 * Initializes a game with count of rooms in the labyrinth and seed number.
 */
void game_init (Game *game, int rows, int cols, int seed);

void game_run_loop (Game *game, Render *render);

/* ========== THIS FUNCTION RUNTIME DEPENDED ========== */

enum command read_command (Game *game);

int handle_command (Game *game, enum command cmd);

void render (Render *render, Game *game);

Menu *create_menu (enum game_state state);

void close_menu (Menu *menu, enum game_state state);

#endif /* __LABYRINTH_GAME__ */
