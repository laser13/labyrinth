/**
 *  This is terminal runtime for the game.
 *  Here we have implementations of all runtime-depended
 *  functions of the game, such render or handling key pressing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "dbuf.h"
#include "game.h"
#include "term.h"

// clang-format off
#define WELCOME_SCREEN  \
"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n" \
"░░░██╗░░░░░░█████╗░██████╗░██╗░░░██╗██████╗░██╗███╗░░██╗████████╗██╗░░██╗░░\n" \
"░░░██║░░░░░██╔══██╗██╔══██╗╚██╗░██╔╝██╔══██╗██║████╗░██║╚══██╔══╝██║░░██║░░\n" \
"░░░██║░░░░░███████║██████╦╝░╚████╔╝░██████╔╝██║██╔██╗██║░░░██║░░░███████║░░\n" \
"░░░██║░░░░░██╔══██║██╔══██╗░░╚██╔╝░░██╔══██╗██║██║╚████║░░░██║░░░██╔══██║░░\n" \
"░░░███████╗██║░░██║██████╦╝░░░██║░░░██║░░██║██║██║░╚███║░░░██║░░░██║░░██║░░\n" \
"░░░╚══════╝╚═╝░░╚═╝╚═════╝░░░░╚═╝░░░╚═╝░░╚═╝╚═╝╚═╝░░╚══╝░░░╚═╝░░░╚═╝░░╚═╝░░\n" \
"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n" \
"░░                           ┃   ┃   ┃       ┃   ┃   ┃   ┃   ┃       ┃   ░░\n" \
"░░━━━━━━━┳━━━    ┃   ┏━━━┳━━━┛       ┣━━━            ┃   ┃   ┗━━━┓       ░░\n" \
"░░       ┃       ┃   ┃   ┃           ┃               ┃   ┃       ┃       ░░\n" \
"░░━━━┓       ━━━━┫       ┣━━━┳━━━    ┗━━━━━━━━━━━    ┃   ┣━━━┓   ┣━━━┓   ░░\n" \
"░░   ┃           ┃       ┃   ┃                       ┃   ┃   ┃   ┃   ┃   ░░\n" \
"░░   ┗━━━━━━━┓   ┃   ━━━━┫   ┃   ━━━━━━━━━━━━━━━━┳━━━┛                   ░░\n" \
"░░           ┃   ┃       ┃   ┃                   ┃                       ░░\n" \
"░░━━━━━━━    ┃   ┗━━━┓       ┗━━━┳━━━━━━━━━━━    ┗━━━━━━━┳━━━━━━━━━━━━   ░░\n" \
"░░           ┃       ┃           ┃                       ┃               ░░\n" \
"░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░" 

#define NEW_GAME \
"█▄░█ █▀▀ █░█░█   █▀▀ ▄▀█ █▀▄▀█ █▀▀\n" \
"█░▀█ ██▄ ▀▄▀▄▀   █▄█ █▀█ █░▀░█ ██▄"

// clang-format on

typedef struct
{
  int state;
  dbuf frame;
} screen;

#define SCREEN_EMPTY                                                          \
  {                                                                           \
    0, 0, DBUF_EMPTY                                                          \
  }

/* The count of symbols by vertical of one room.  */
static const int laby_room_rows = 2;

/* The count of symbols by horizontal of one room.  */
static const int laby_room_cols = 4;

static int
expect_borders (int border, char expected)
{
  return (border & expected) == expected;
}

static int
not_expect_borders (int border, char expected)
{
  return (border & expected) == 0;
}

static char *
get_corner (int border, int neighbor)
{
  if (expect_borders (border, LEFT_BORDER | UPPER_BORDER)
      && not_expect_borders (neighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return "┏";
  if (expect_borders (border, LEFT_BORDER | UPPER_BORDER)
      && expect_borders (neighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return "╋";
  if (expect_borders (border, LEFT_BORDER | UPPER_BORDER)
      && expect_borders (neighbor, RIGHT_BORDER))
    return "┣";
  if (expect_borders (border, LEFT_BORDER | UPPER_BORDER)
      && expect_borders (neighbor, BOTTOM_BORDER))
    return "┳";
  if (expect_borders (border, LEFT_BORDER)
      && expect_borders (neighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return "┫";
  if (expect_borders (border, UPPER_BORDER)
      && expect_borders (neighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return "┻";
  if (expect_borders (border, LEFT_BORDER | UPPER_BORDER)
      && not_expect_borders (neighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return "┏";
  if (not_expect_borders (border, LEFT_BORDER | UPPER_BORDER)
      && expect_borders (neighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return "┛";
  if (expect_borders (border, UPPER_BORDER)
      && expect_borders (neighbor, RIGHT_BORDER))
    return "┗";
  if (expect_borders (border, LEFT_BORDER)
      && expect_borders (neighbor, BOTTOM_BORDER))
    return "┓";
  if (expect_borders (border, UPPER_BORDER)
      && not_expect_borders (neighbor, RIGHT_BORDER))
    return "━";
  if (expect_borders (border, LEFT_BORDER)
      && not_expect_borders (neighbor, BOTTOM_BORDER))
    return "┃";

  return " ";
}

static char *
get_creature (level *level, int r, int c)
{
  if (level->player.r == r && level->player.c == c)
    return "@";
  else
    return 0;
}

static void
render_room (level *level, int r, int c, int i, int j, dbuf *buf)
{
  laby *lab = &level->lab;

  /* We will render left and upper borders at once.
   * To choose correct symbol for the corner we need to know a
   * neighbor. */
  int border = laby_get_border (lab, r, c);
  int neighbor = laby_get_border (lab, r - 1, c - 1);

  int is_visited = laby_is_visited (lab, r, c);
  char *creature = get_creature (level, r, c);

  char *s;
  /* render the first row of the room */
  if (i == 0)
    {
      s = (j == 0)                  ? get_corner (border, neighbor)
          : (border & UPPER_BORDER) ? "━"
          : (is_visited || laby_is_visited (lab, r - 1, c)) ? "·"
                                                            : " ";
    }
  /* render the content of the room (the second row) */
  else
    {
      int is_border = (j == 0) && (border & LEFT_BORDER);

      s = (is_border)                       ? "┃"
          : (creature && (i > 0 && j == 2)) ? creature
          : (is_visited)                    ? "·"
                                            : " ";
    }
  buffer_append_str (buf, s, strlen (s));
}

/**
 * Renders the labyrinth `lab` to the buffer `buf`.
 */
void
render_level (level *level, dbuf *buf)
{
  laby *lab = &level->lab;

  /* Render rooms from every row, plus one extra row for the bottom borders */
  for (int r = 0; r <= lab->rows_count; r++)
    {
      /* iterates over room height (only one line for the last extra row) */
      int n = (r < lab->rows_count) ? laby_room_rows : 1;
      for (int i = 0; i < n; i++)
        {
          /* Take a room from the row, plus one more for the right border */
          for (int c = 0; c <= lab->cols_count; c++)
            {

              /* Iterate over columns of the single room
               * (only one symbol for the extra right room) */
              int k = (c < lab->cols_count) ? laby_room_cols : 1;
              for (int j = 0; j < k; j++)
                {
                  render_room (level, r, c, i, j, buf);
                }
            }
          if (r < lab->rows_count)
            buffer_end_line (buf);
        }
    }
}

void
render_welcome_screen (screen *s, dbuf *buf)
{
  // double fps = 0.3;
  /* Do nothing if processing happens too often */
  // if (difftime(time(NULL), s->last_update) < fps)
  for (int i = 0; i < s->frame.lines_count; i++)
    buffer_add_line (buf, s->frame.lines[i].chars, s->frame.lines[i].length);
}

void
render (game *game, double time_frame_s)
{
  /* Put the cursor to the upper left corner */
  dbuf buf;
  buffer_init (&buf, CUP);
  switch (game->state)
    {
    case ST_MAIN_MENU:
      render_welcome_screen ((screen *)game->menu, &buf);
      break;
    default:
      render_level (&game->level, &buf);
    }
  buffer_write (STDIN_FILENO, &buf);
  buffer_free (&buf);
}

void *
create_menu (const game *game, enum game_state state)
{
  // switch (state)
  //   {
  //   case ST_MAIN_MENU:
  screen *welcome_screen = malloc (sizeof (screen));
  welcome_screen->state = 1;
  buffer_parse (&welcome_screen->frame, WELCOME_SCREEN);
  return welcome_screen;
  // }
}

void
close_menu (void *menu, enum game_state state)
{
  screen *welcome_screen = (screen *)menu;
  buffer_free (&welcome_screen->frame);
  free (welcome_screen);
}

enum command
read_command (game *game)
{
  key_p key = read_key ();
  switch (game->state)
    {
    case ST_MAIN_MENU:
      if (key.len == 1 && key.chars[0] == ENTER_KEY)
        return CMD_NEW_GAME;
      if (key.len == 1 && key.chars[0] == ESC)
        return CMD_EXIT;

    case ST_GAME:
      if (key.len == 1 && key.chars[0] == ESC)
        return CMD_EXIT;
      if (key.len == 3 && key.chars[1] == '[')
        {
          switch (key.chars[2])
            {
            case 'A':
              return CMD_MV_UP;
            case 'B':
              return CMD_MV_DOWN;
            case 'C':
              return CMD_MV_RIGHT;
            case 'D':
              return CMD_MV_LEFT;
            }
        }

    default:
      return CMD_NOTHING;
    }
}
