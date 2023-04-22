#include "dstr.c"
#include "laby.c"
#include <stdio.h>

/**
 * Renders a labyrinth `lab` to the buffer `buf` in compact way.
 */
void
uc_render_laby_compact (dstr *buf, laby *lab)
{
  char *room = "═╬═";
  for (int r = 0; r < lab->rows_count; r++)
    {
      for (int c = 0; c < lab->cols_count; c++)
        {
          switch (laby_get_border (lab, r, c))
            {
            case (LEFT_BORDER | UPPER_BORDER | RIGHT_BORDER | BOTTOM_BORDER):
              room = "   ";
              break;
            case LEFT_BORDER:
              room = " ╠═";
              break;
            case RIGHT_BORDER:
              room = "═╣ ";
              break;
            case UPPER_BORDER:
              room = "═╦═";
              break;
            case BOTTOM_BORDER:
              room = "═╩═";
              break;
            case (UPPER_BORDER | BOTTOM_BORDER):
              room = "═══";
              break;
            case (LEFT_BORDER | RIGHT_BORDER):
              room = " ║ ";
              break;
            case (LEFT_BORDER | UPPER_BORDER):
              room = " ╔═";
              break;
            case (RIGHT_BORDER | UPPER_BORDER):
              room = "═╗ ";
              break;
            case (LEFT_BORDER | BOTTOM_BORDER):
              room = " ╚═";
              break;
            case (RIGHT_BORDER | BOTTOM_BORDER):
              room = "═╝ ";
              break;
            case (LEFT_BORDER | UPPER_BORDER | RIGHT_BORDER):
              room = " ╓ ";
              break;
            case (UPPER_BORDER | RIGHT_BORDER | BOTTOM_BORDER):
              room = "╕  ";
              break;
            case (RIGHT_BORDER | BOTTOM_BORDER | LEFT_BORDER):
              room = " ╙ ";
              break;
            case (BOTTOM_BORDER | LEFT_BORDER | UPPER_BORDER):
              room = "  ╘";
              break;
            default:
              room = "═╬═";
              break;
            }
          dstr_append (buf, room, strlen (room));
        }
      if (r < lab->rows_count - 1)
        dstr_append (buf, "\r\n", 2);
    }
}

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
uc_get_coner (int border, int neighbor)
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

/**
 * Renders the labyrinth `lab` to the buffer `buf`.
 *
 * @param r_rows the count of symbols by vertical of one room. Min 2.
 * @param c_rows the count of symbols by horizontal of one room. Min 2.
 */
void
uc_render_laby (dstr *buf, laby *lab, int r_rows, int r_cols)
{
  assert (r_rows > 0 && r_cols > 0);

  char *s;
  /* Render rooms from every row, plus one extra row for the bottom borders */
  for (int r = 0; r <= lab->rows_count; r++)
    {
      /* iterates over room height (only one line for the last extra row) */
      int n = (r < lab->rows_count) ? r_rows : 1;
      for (int i = 0; i < n; i++)
        {
          /* Take a room form the row, plus one more for the right border */
          for (int c = 0; c <= lab->cols_count; c++)
            {
              /* We will render left and upper borders at once.
               * To choose correct symbol for the corner we need to know a
               * neighbor. */
              int border = laby_get_border (lab, r, c);
              int neighbor = laby_get_border (lab, r - 1, c - 1);

              /* Iterate over width of the single room
               * (only one symbol for the extra right room) */
              int k = (c < lab->cols_count) ? r_cols : 1;
              for (int j = 0; j < k; j++)
                {
                  /* render the first row of the room */
                  if (i == 0)
                    {
                      s = (j == 0) ? uc_get_coner (border, neighbor)
                          : (border & UPPER_BORDER) ? "━"
                                                    : " ";
                    }
                  /* render others */
                  else
                    {
                      s = (j == 0 && (border & LEFT_BORDER)) ? "┃" : " ";
                    }
                  dstr_append (buf, s, strlen (s));
                }
            }
          if (r < lab->rows_count)
            dstr_append (buf, "\r\n", 2);
        }
    }
}
