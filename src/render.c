/**
 *  We will "draw" a game to the two dimension byte array as
 * indexes of the array with utf-8 symbols. It should help to avoid issues with
 * different size of symbols in utf-8 encoding. And at the end, we will
 * "render" this symbols map to the u8 buffer.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "2d_math.h"
#include "art.h"
#include "game.h"
#include "laby.h"
#include "render.h"
#include "u8.h"

// clang-format off
/* The array of symbols, which are used to render the game. */
static const char *symbols[] = 
//   0    1    2    3    4    5    6    7    8    9
  { 
     "", " ", "┃", "━", "┏", "┓", "┗", "┛", "╋", "┣",
    "┫", "┳", "┻", "@", "⛿", "·", "*" 
  };
// clang-format on

void
smap_init (smap *sm, int rows, int cols, int room_height, int room_width)
{
  sm->height = rows * room_height + 1;
  sm->width = cols * room_width + 1;
  sm->symbols = malloc (sizeof (symbol *) * sm->height);
  for (int i = 0; i < sm->height; i++)
    sm->symbols[i] = malloc (sizeof (symbol) * sm->width);
}

static inline void
symbols_map_draw (smap *sm, double y, double x, symbol s)
{
  int ix = round (x);
  int iy = round (y);
  if (ix < 0 || iy < 0)
    return;
  if (ix >= sm->width || iy >= sm->height)
    return;
  sm->symbols[iy][ix] = s;
}

void
smap_free (smap *sm)
{
  for (int i = 0; i < sm->height; i++)
    free (sm->symbols[i]);
  free (sm->symbols);
}

static void
create_frame (u8buf *buf, int height, int width)
{
  for (int i = 0; i < height; i++)
    {
      u8str str = U8_STR_EMPTY;
      if (i == 0)
        {
          u8_str_append_str (&str, symbols[4]);
          u8_str_append_repeate_str (&str, symbols[3], width - 2);
          u8_str_append_str (&str, symbols[5]);
        }
      else if (i == height - 1)
        {
          u8_str_append_str (&str, symbols[6]);
          u8_str_append_repeate_str (&str, symbols[3], width - 2);
          u8_str_append_str (&str, symbols[7]);
        }
      else
        {
          u8_str_append_str (&str, symbols[2]);
          u8_str_append_repeate_str (&str, symbols[1], width - 2);
          u8_str_append_str (&str, symbols[2]);
        }
      u8_buffer_add_line (buf, str.chars, str.length);
    }
}

static symbol
get_corner (int broom, int bneighbor)
{
  if (EXPECT_BORDERS (broom, LEFT_BORDER | UPPER_BORDER)
      && EXPECT_BORDERS (bneighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return 8; // "╋";
  if (EXPECT_BORDERS (broom, LEFT_BORDER | UPPER_BORDER)
      && EXPECT_BORDERS (bneighbor, RIGHT_BORDER))
    return 9; // "┣"
  if (EXPECT_BORDERS (broom, LEFT_BORDER | UPPER_BORDER)
      && EXPECT_BORDERS (bneighbor, BOTTOM_BORDER))
    return 11; // "┳"
  if (EXPECT_BORDERS (broom, LEFT_BORDER)
      && EXPECT_BORDERS (bneighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return 10; // "┫"
  if (EXPECT_BORDERS (broom, UPPER_BORDER)
      && EXPECT_BORDERS (bneighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return 12; // "┻"
  if (EXPECT_BORDERS (broom, LEFT_BORDER | UPPER_BORDER)
      && NOT_EXPECT_BORDERS (bneighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return 4; // "┏"
  if (NOT_EXPECT_BORDERS (broom, LEFT_BORDER | UPPER_BORDER)
      && EXPECT_BORDERS (bneighbor, RIGHT_BORDER | BOTTOM_BORDER))
    return 7; // "┛"
  if (EXPECT_BORDERS (broom, UPPER_BORDER)
      && EXPECT_BORDERS (bneighbor, RIGHT_BORDER))
    return 6; // "┗"
  if (EXPECT_BORDERS (broom, LEFT_BORDER)
      && EXPECT_BORDERS (bneighbor, BOTTOM_BORDER))
    return 5; // "┓"
  if (EXPECT_BORDERS (broom, UPPER_BORDER)
      && NOT_EXPECT_BORDERS (bneighbor, RIGHT_BORDER))
    return 3; // "━"
  if (EXPECT_BORDERS (broom, LEFT_BORDER)
      && NOT_EXPECT_BORDERS (bneighbor, BOTTOM_BORDER))
    return 2; // "┃"

  return 0;
}

void
draw_in_the_middle_of_room (smap *sm, int r, int c, symbol s)
{
  int y = r * laby_room_height + (laby_room_height / 2);
  int x = c * laby_room_width + (laby_room_width / 2);
  symbols_map_draw (sm, y, x, s);
}

/**
 * @r number of the room by vertical
 * @c number of the room by horizontal
 * @y number of the char inside the room by vertical
 * @x number of the char inside the room by horizontal
 */
static void
draw_room (smap *sm, Laby *lab, int r, int c, int y, int x)
{
  /* We will render left and upper borders at once.
   * To choose correct symbol for the corner we need to know a
   * neighbor. */
  int border = laby_get_borders (lab, r, c);
  int neighbor = laby_get_borders (lab, r - 1, c - 1);

  symbol *idx
      = &sm->symbols[y + r * laby_room_height][x + c * laby_room_width];

  /* render the first row of symbols of the room */
  if (y == 0)
    {
      *idx = (x == 0) ? get_corner (border, neighbor) : 0;
      *idx = (*idx)                              ? *idx
             : (border & UPPER_BORDER)           ? 3
             : (laby_is_visible (lab, r, c))     ? SIDX_LIGHT
             : (laby_is_visible (lab, r - 1, c)) ? SIDX_LIGHT
                                                 : SIDX_EMPTY;
    }
  /* render the content of the room (the second row) */
  else
    {
      *idx = ((x == 0) && (border & LEFT_BORDER)) ? 2
             : (laby_is_visible (lab, r, c))      ? SIDX_LIGHT
                                                  : SIDX_EMPTY;
    }
}

void
draw_laby (smap *sm, Laby *lab)
{
  /* Render rooms from every row, plus one extra row for the bottom borders */
  for (int r = 0; r <= lab->rows; r++)
    {
      /* iterates over room height (only one line for the last extra row) */
      int rh = (r < lab->rows) ? laby_room_height : 1;
      for (int ry = 0; ry < rh; ry++)
        {
          /* Take a room from the row, plus one more for the right border */
          for (int c = 0; c <= lab->cols; c++)
            {
              /* Iterate over columns of the single room
               * (only one symbol for the extra right room) */
              int rw = (c < lab->cols) ? laby_room_width : 1;
              for (int rx = 0; rx < rw; rx++)
                draw_room (sm, lab, r, c, ry, rx);
            }
        }
    }
}

void
render_symbols_map (u8buf *dest, const smap *source)
{
  for (int i = 0; i < source->height; i++)
    {
      for (int j = 0; j < source->width; j++)
        {
          const char *s = symbols[source->symbols[i][j]];
          u8_buffer_append_str (dest, s, strlen (s));
        }
      u8_buffer_end_line (dest);
    }
}

void
render_game (u8buf *buf, Game *game)
{
  smap sm;
  smap_init (&sm, game->lab.rows, game->lab.cols, laby_room_height,
             laby_room_width);
  laby_mark_visible_rooms (&game->lab, game->player.row, game->player.col,
                           game->player.visible_range);
  draw_laby (&sm, &game->lab);
  draw_in_the_middle_of_room (&sm, game->exit.row, game->exit.col, SIDX_EXIT);
  draw_in_the_middle_of_room (&sm, game->player.row, game->player.col,
                              SIDX_PLAYER);
  render_symbols_map (buf, &sm);
  smap_free (&sm);
}

void
render_welcome_screen (u8buf *buf, void *menu)
{
  Menu *m = (Menu *)menu;
  /* Blink menu option */
  time_t now = time (NULL);
  if ((now - m->last_update_at) > 0.4)
    {
      // m->state = -m->state;
      m->last_update_at = now;
    }

  u8_buffer_parse (buf, WELCOME_SCREEN);
  u8buf label = U8_BUF_EMPTY;
  switch (m->state)
    {
    case 0:
      m->state = 1;
      break;
      /* New game */
    case 1:
      u8_buffer_parse (&label, LB_NEW_GAME);
      u8_buffer_merge (buf, &label, 14, 22);
      break;
      /* Exit */
    case 2:
      u8_buffer_parse (&label, LB_EXIT);
      u8_buffer_merge (buf, &label, 14, 30);
      break;
    }
  u8_buffer_free (&label);
}

void
render_pause_menu (u8buf *buf, void *menu)
{
  Menu *m = (Menu *)menu;
  u8buf frame = U8_BUF_EMPTY;
  u8buf label = U8_BUF_EMPTY;
  create_frame (&frame, 8, 40);
  switch (m->state)
    {
    case 1:
      u8_buffer_parse (&label, LB_CONTINUE);
      u8_buffer_merge (&frame, &label, 3, 4);
      break;
    case 0:
      u8_buffer_parse (&label, LB_EXIT);
      u8_buffer_merge (&frame, &label, 3, 14);
      break;
    }
  u8_buffer_merge (buf, &frame, 8, 19);
  u8_buffer_free (&frame);
  u8_buffer_free (&label);
}

void
render_winning (u8buf *buf, void *menu)
{
  u8buf frame = U8_BUF_EMPTY;
  u8buf label = U8_BUF_EMPTY;
  create_frame (&frame, 10, 60);
  u8_buffer_parse (&label, LB_YOU_WIN);
  u8_buffer_merge (&frame, &label, 2, 2);
  u8_buffer_merge (buf, &frame, 6, 8);
  u8_buffer_free (&frame);
  u8_buffer_free (&label);
}

void
render (Game *game)
{
  u8buf buf = U8_BUF_EMPTY;

  /* only the main screen overlaps the level completely,
   * in all other cases we should render the laby before anything else. */
  if (game->state != ST_MAIN_MENU)
    render_game (&buf, game);

  switch (game->state)
    {
    case ST_MAIN_MENU:
      render_welcome_screen (&buf, game->menu);
      break;
    case ST_PAUSE:
      render_pause_menu (&buf, game->menu);
      break;
    case ST_WIN:
      render_winning (&buf, game->menu);
      break;
    case ST_GAME:
      break;
    }

  int rowpad = (screen_rows - game_window_rows) / 2;
  rowpad = (rowpad > 0) ? rowpad : 0;
  int colpad = (screen_cols - game_window_cols) / 2;
  colpad = (colpad > 0) ? colpad : 0;

  int rows = (screen_rows < game_window_rows) ? screen_rows : game_window_rows;
  int cols = (screen_cols < game_window_cols) ? screen_cols : game_window_cols;

  u8_buffer_write (STDIN_FILENO, &buf, rowpad, colpad, rows, cols);
  u8_buffer_free (&buf);
}
