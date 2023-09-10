#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "curses.h"

#define __PAGES_VARS_IMPLEMENTATION__
#include <pages.h>

// side bar defs ------------------------------
typedef struct side_bar_pos_s
{
  uint32_t menu_pos, submenu_pos;
} side_bar_pos;

#define SIDE_BAR_WIDTH (30)

typedef char side_menu_text[SIDE_BAR_WIDTH];

typedef struct choice_s
{
  side_menu_text title;
  side_menu_text *submenus;
  uint32_t n_submenus;

  uint32_t *page_ids;
} choice;

typedef struct side_bar_s
{
  choice choices[PAGE_TOTAL];
  uint32_t startx, starty;
  uint8_t is_focused, is_in_submenu;
  side_bar_pos pos;
} side_bar;

#define LIST_OF_PAGES_NAMES                                                          \
  X(auto_test, 3, "Statistiques", "Vocab", "questions longues"),                     \
      X(declinaison, 3, "Statistiques", "Cas -> Declinaison", "Declinaison -> Cas"), \
      X(conjugaison, 3, "Statistiques", "Pers -> Conjugaison", "Conjugaison -> Pers")

// learning defs -----------------------------

typedef struct qt_struct
{
  char *question;
  char *ans;
  struct tm start;
  uint8_t success;
} qt;

const uint32_t intervals[] = {1, 3, 7, 14, 30, 3 * 30, 6 * 30}; // intervals for when questions will be asked, in days

struct tm get_time(void);
void print_date(const struct tm tm);
void print_menu(WINDOW *menu_win, side_bar *side);
int handle_input_side_bar(int ch, side_bar *side);
void move_menu_cursor(side_bar *side, int offset);
void draw_page(WINDOW *win, uint32_t page_idx, const side_bar side);

#define PAIR_SELECTED_VAL (1)
#define PAIR_SELECTED COLOR_PAIR(PAIR_SELECTED_VAL)

int main(int argc, char **argv)
{
  (void)argc, (void)argv;

  // const struct tm tm = get_time();

  side_bar side;
#define X(name, num, ...) name##_submenu[] = {__VA_ARGS__}
  side_menu_text LIST_OF_PAGES_NAMES;
#undef X

#define X(name, num, ...) {.title = #name, .submenus = name##_submenu, .n_submenus = num}

  side = (side_bar){.choices = {
                        LIST_OF_PAGES_NAMES,
                        {.title = "Quit"}},
                    .is_focused = 1,
                    .is_in_submenu = 0,
                    .startx = 0,
                    .starty = 0,
                    .pos = {.menu_pos = 0, .submenu_pos = 0}};

  initscr();
  if (has_colors() == FALSE)
  {
    endwin();
    printf("Your terminal does not support color\n");
    return EXIT_FAILURE;
  }

  start_color(); /* Start color 			*/
  init_pair(PAIR_SELECTED_VAL, COLOR_RED, COLOR_BLACK);

  clear();
  noecho();
  cbreak();
  WINDOW *side_bar = newwin(LINES, SIDE_BAR_WIDTH, side.starty, side.startx);
  WINDOW *page_win = newwin(LINES, COLS - SIDE_BAR_WIDTH, 0, SIDE_BAR_WIDTH);

  // box(page_win, 0, 0);
  // wrefresh(page_win);
  // print_menu(side_bar, &side);

  uint32_t page_idx = 0;
  while (1)
  {
    wrefresh(page_win);
    print_menu(side_bar, &side);

    int ch = wgetch(side_bar);

    // handle the side bar
    if (side.is_focused)
    {
      page_idx = handle_input_side_bar(ch, &side);

      if (page_idx == QUIT)
        break;

      continue;
    }

    if (ch == 27)
      side.is_focused = 1;

    draw_page(page_win, page_idx, side);
  }

  endwin();

  free(side_bar);
  free(page_win);

  return EXIT_SUCCESS;
}

struct tm get_time(void)
{
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  tm.tm_year += 1900;
  tm.tm_mon += 1;

  return tm;
}

void print_date(const struct tm tm)
{
  printf("%02u/%02u/%u", tm.tm_mday, tm.tm_mon, tm.tm_year);

  return;
}

void print_menu(WINDOW *menu_win, side_bar *side)
{
  wclear(menu_win);
  int x, y;

  x = 2;
  y = 2;
  wborder(menu_win, '|', '|', '-', '-', '+', '+', '+', '+');
  for (uint32_t i = 0; i < PAGE_TOTAL; ++i, ++y)
  {
    if (side->pos.menu_pos == i)
      wattron(menu_win, A_REVERSE);

    mvwprintw(menu_win, y, x, "%s", side->choices[i].title);

    if (side->pos.menu_pos == i)
    {
      wattroff(menu_win, A_REVERSE);

      ++y;
      for (uint32_t j = 0; j < side->choices[i].n_submenus; ++j, ++y)
      {
        uint8_t should_be_selected = side->pos.submenu_pos == j && side->is_in_submenu;
        uint8_t should_be_colored = (side->pos.submenu_pos == j && ((!side->is_focused && !side->is_in_submenu) || (side->is_in_submenu)));
        mvwprintw(menu_win, y, x, "%s ", should_be_selected ? "->" : " +");

        wattron(menu_win, should_be_colored ? PAIR_SELECTED : 0);
        mvwprintw(menu_win, y, x + 3, "%s", side->choices[i].submenus[j]);
        wattroff(menu_win, should_be_colored ? PAIR_SELECTED : 0);
      }
    }
  }
  wrefresh(menu_win);
}

int handle_input_side_bar(int ch, side_bar *side)
{
  switch (ch)
  {
  case 'j':
    move_menu_cursor(side, 1);
    break;

  case 'k':
    move_menu_cursor(side, -1);
    break;

  case 'h':
    side->is_in_submenu = 0;
    break;

  case 10:
    if (side->is_in_submenu)
      side->is_in_submenu = side->is_focused = 0;
    else
      side->is_in_submenu = 1;

    return side->pos.menu_pos;

  default:
    break;
  }

  return 0;
}

void move_menu_cursor(side_bar *side, int offset)
{
  if (side->is_in_submenu)
  {
    if ((side->pos.submenu_pos != 0 || offset >= 0) && side->pos.submenu_pos + offset < side->choices[side->pos.menu_pos].n_submenus)
      side->pos.submenu_pos += offset;
  }
  else // not in submenu
  {
    if ((side->pos.menu_pos != 0 || offset >= 0) && side->pos.menu_pos + offset < PAGE_TOTAL)
    {
      side->pos.menu_pos += offset;
      side->pos.submenu_pos = 0;
    }
  }

  return;
}

void draw_page(WINDOW *win, uint32_t page_idx, const side_bar side)
{
  wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
  for (uint32_t i = 0; i < MAX_TEXT_HEIGHT; ++i)
    mvwprintw(win, i + 1, 1, "%s", pages[page_idx][side.pos.submenu_pos][i]);
}