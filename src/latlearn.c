#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "curses.h"

uint32_t startx = 0, starty = 0;

const char *choices[] = {"Auto-test",
                         "Conjugaison",
                         "Declinaison",
                         "Quit"};

enum
{
  PAGE_AUTO_TEST,
  PAGE_CONJUGATE,
  PAGE_DECLINATE,
  QUIT,
  PAGE_TOTAL
};

const int n_choices = PAGE_TOTAL;

typedef struct qt_struct
{
  char *question;
  char *ans;
  struct tm start;
  uint8_t success;
} qt;

#define SIDE_BAR_WIDTH (30)

const uint32_t intervals[] = {1, 3, 7, 14, 30, 3 * 30, 6 * 30}; // intervals for when questions will be asked, in days

struct tm get_time(void);
void print_date(const struct tm tm);
void print_menu(WINDOW *menu_win, uint32_t highlight);
int handle_input_side_bar(int ch, int *highlight);

int main(int argc, char **argv)
{
  (void)argc, (void)argv;

  // const struct tm tm = get_time();

  uint8_t is_in_side_bar = 1;

  initscr();
  clear();
  noecho();
  cbreak();
  WINDOW *side_bar = subwin(stdscr, LINES, SIDE_BAR_WIDTH, 0, 0);

  side_bar = newwin(LINES, SIDE_BAR_WIDTH, starty, startx);
  print_menu(side_bar, 1);

  int page = 0, highlight = 1;
  while (1)
  {
    int ch = wgetch(side_bar);
    if (is_in_side_bar)
    {
      page = handle_input_side_bar(ch, &highlight);
      if (page)
        is_in_side_bar = 0;

      if (page == 4)
        break;
    }
    else
    {
      if (ch == 'h')
        is_in_side_bar = 1;
    }

    switch (page)
    {
    case PAGE_AUTO_TEST:
      print_auto_test();
      break;

    case PAGE_CONJUGATE:
      print_conjugate();
      break;

    case PAGE_DECLINATE:
      print_declinate();
      break;

    default:
      fprintf(stderr, "[ERROR] Unreachable !!\n");
      assert(0 && "unreachable");
      break;
    }

    print_menu(side_bar, is_in_side_bar ? highlight : 0);
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

void print_menu(WINDOW *menu_win, uint32_t highlight)
{
  int x, y;

  x = 2;
  y = 2;
  box(menu_win, 0, 0);
  for (uint32_t i = 0; i < n_choices; ++i)
  {
    if (highlight == i + 1)
    {
      wattron(menu_win, A_REVERSE);
      mvwprintw(menu_win, y + i, x, "%s", choices[i]);
      wattroff(menu_win, A_REVERSE);
    }
    else
      mvwprintw(menu_win, y + i, x, "%s", choices[i]);
  }
  wrefresh(menu_win);
}

int handle_input_side_bar(int ch, int *highlight)
{
  switch (ch)
  {
  case 'j':
    *highlight = (*highlight >= n_choices) ? 1 : *highlight + 1;
    break;

  case 'k':
    *highlight = (*highlight <= 1) ? n_choices : *highlight - 1;
    break;

  case 10:
    return *highlight;

  default:
    break;
  }

  return 0;
}
