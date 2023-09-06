#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

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

int main(int argc, char **argv)
{
  (void)argc, (void)argv;

  const struct tm tm = get_time();

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
