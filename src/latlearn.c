#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "curses.h"

// side bar defs ------------------------------
uint32_t startx = 0, starty = 0;

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
  uint32_t n_entrys_submenu;
} choice;

enum
{
  PAGE_AUTO_TEST,
  PAGE_CONJUGATE,
  PAGE_DECLINATE,
  QUIT,
  PAGE_TOTAL
};

#define MOVE_MENU_CURSOR(pos, offset)                                                                                   \
  do                                                                                                                    \
  {                                                                                                                     \
    if (!is_in_submenu)                                                                                                 \
    {                                                                                                                   \
      if ((pos).menu_pos + offset >= 1 && (pos).menu_pos + offset <= n_choices)                                         \
        (pos).menu_pos += offset;                                                                                       \
    }                                                                                                                   \
    else if ((pos).submenu_pos + offset >= 1 && (pos).submenu_pos + offset <= choices[(pos).menu_pos].n_entrys_submenu) \
      (pos).submenu_pos += offset;                                                                                      \
  } while (0);

#define LIST_OF_PAGES(...)                                                   \
  X(auto_test, "Statistiques", "Francais -> Latin", "Latin -> Francais")     \
  __VA_OPT__(, )                                                             \
  X(declinaison, "Statistiques", "Cas -> Declinaison", "Declinaison -> Cas") \
  __VA_OPT__(, )                                                             \
  X(conjugaison, "Statistiques", "Pers -> Conjugaison", "Conjugaison -> Pers")

#define X(name, ...) side_menu_text name##_submenu[] = {__VA_ARGS__};
LIST_OF_PAGES()
#undef X

#define X(name, ...)                                                                                                   \
  {                                                                                                                    \
    .title = #name, .submenus = name##_submenu, .n_entrys_submenu = sizeof(name##_submenu) / sizeof(name##_submenu[0]) \
  }

const choice choices[PAGE_TOTAL] = {
    LIST_OF_PAGES(a),
    {.title = "Quit"}};
#undef X

const int n_choices = PAGE_TOTAL;

uint8_t is_in_side_bar = 1, is_in_submenu = 0;

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
void print_menu(WINDOW *menu_win, side_bar_pos *pos);
int handle_input_side_bar(int ch, side_bar_pos *pos);

int main(int argc, char **argv)
{
  (void)argc, (void)argv;

  // const struct tm tm = get_time();

  initscr();
  clear();
  noecho();
  cbreak();
  WINDOW *side_bar = newwin(LINES, SIDE_BAR_WIDTH, starty, startx);
  print_menu(side_bar, &(side_bar_pos){1, 1});

  int page = 0;
  side_bar_pos pos = {.menu_pos = 1, .submenu_pos = 1};
  while (1)
  {
    int ch = wgetch(side_bar);
    if (is_in_side_bar)
    {
      page = handle_input_side_bar(ch, &pos);

      if (page == 4)
        break;
    }

    if (ch == 'h')
    {
      if (is_in_side_bar)
        is_in_submenu = 0;
      else
        is_in_submenu = 1;

      is_in_side_bar = 1;
    }

    print_menu(side_bar, &pos);
    mvprintw(0, 50, "%u, %u", pos.menu_pos, pos.submenu_pos);
  }

  endwin();

  free(side_bar);

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

void print_menu(WINDOW *menu_win, side_bar_pos *pos)
{
  wclear(menu_win);
  int x, y;

  x = 2;
  y = 2;
  box(menu_win, 0, 0);
  for (uint32_t i = 0; i < n_choices; ++i, ++y)
  {
    if (pos->menu_pos == i + 1)
      wattron(menu_win, A_REVERSE);

    mvwprintw(menu_win, y, x, "%s", choices[i].title);

    if (pos->menu_pos == i + 1)
    {
      wattroff(menu_win, A_REVERSE);

      ++y;
      for (uint32_t j = 0; j < choices[i].n_entrys_submenu; ++j, ++y)
        mvwprintw(menu_win, y, x, "%s %s", pos->submenu_pos == j + 1 && is_in_submenu ? "->" : " +", choices[i].submenus[j]);
    }
  }
  wrefresh(menu_win);
}

int handle_input_side_bar(int ch, side_bar_pos *pos)
{
  switch (ch)
  {
  case 'j':
    // *highlight = (*highlight >= n_choices) ? 1 : *highlight + 1;
    MOVE_MENU_CURSOR(*pos, 1);
    break;

  case 'k':
    // *highlight = (*highlight <= 1) ? n_choices : *highlight - 1;
    MOVE_MENU_CURSOR(*pos, -1);
    break;

  case 10:
    if (is_in_submenu)
      is_in_submenu = is_in_side_bar = 0;
    else
      is_in_submenu = 1;

    return pos->menu_pos;

  default:
    break;
  }

  return 0;
}
