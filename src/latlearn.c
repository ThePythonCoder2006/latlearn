#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "curses.h"

// side bar defs ------------------------------
typedef struct side_bar_pos_s
{
  uint32_t menu_pos, submenu_pos;
} side_bar_pos;

#define SIDE_BAR_WIDTH (30)

typedef char side_menu_text[SIDE_BAR_WIDTH];

enum
{
  PAGE_AUTO_TEST,
  PAGE_CONJUGATE,
  PAGE_DECLINATE,
  QUIT,
  PAGE_TOTAL
};

typedef struct choice_s
{
  side_menu_text title;
  side_menu_text *submenus;
  uint32_t n_submenus;
} choice;

typedef struct side_bar_s
{
  choice choices[PAGE_TOTAL];
  uint32_t n_choices;
  uint32_t startx, starty;
  uint8_t is_focused, is_in_submenu;
  side_bar_pos pos;
} side_bar;

#define LIST_OF_PAGES(...)                                                   \
  X(auto_test, "Statistiques", "Francais -> Latin", "Latin -> Francais")     \
  __VA_OPT__(, )                                                             \
  X(declinaison, "Statistiques", "Cas -> Declinaison", "Declinaison -> Cas") \
  __VA_OPT__(, )                                                             \
  X(conjugaison, "Statistiques", "Pers -> Conjugaison", "Conjugaison -> Pers")

#define X(name, ...) side_menu_text name##_submenu[] = {__VA_ARGS__};
LIST_OF_PAGES()
#undef X

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

#define PAIR_SELECTED_VAL (1)
#define PAIR_SELECTED COLOR_PAIR(PAIR_SELECTED_VAL)

int main(int argc, char **argv)
{
  (void)argc, (void)argv;

  // const struct tm tm = get_time();

#define X(name, ...) {.title = #name, .submenus = name##_submenu, .n_submenus = sizeof(name##_submenu) / sizeof(name##_submenu[0])}

  side_bar side = {.choices = {
                       LIST_OF_PAGES(a),
                       {.title = "Quit"}},
                   .is_focused = 1,
                   .is_in_submenu = 0,
                   .n_choices = PAGE_TOTAL,
                   .startx = 0,
                   .starty = 0,
                   .pos = {.menu_pos = 0, .submenu_pos = 0}};
#undef X

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

  box(page_win, 0, 0);
  wrefresh(page_win);
  print_menu(side_bar, &side);

  int page = 0;
  while (1)
  {
    int ch = wgetch(side_bar);
    if (side.is_focused)
    {
      page = handle_input_side_bar(ch, &side);

      if (page == QUIT)
        break;
    }

    if (ch == 27)
      side.is_focused = 1;

    box(page_win, 0, 0);
    wrefresh(page_win);
    print_menu(side_bar, &side);
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
  box(menu_win, 0, 0);
  for (uint32_t i = 0; i < side->n_choices; ++i, ++y)
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
    if ((side->pos.menu_pos != 0 || offset >= 0) && side->pos.menu_pos + offset < side->n_choices)
    {
      side->pos.menu_pos += offset;
      side->pos.submenu_pos = 0;
    }
  }

  return;
}