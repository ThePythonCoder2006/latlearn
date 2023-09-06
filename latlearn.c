#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct tm get_time(void);
void print_date(const struct tm tm);

int main(int argc, char **argv)
{
  (void)argc, (void)argv;

  const struct tm tm = get_time();

  printf("today is the : ");
  print_date(tm);
  putchar('\n');

  printf("Hello latlearn !\n");
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
