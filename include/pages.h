#ifndef __PAGES_H__
#define __PAGES_H__

#include <stdint.h>
#include <inttypes.h>

#ifndef MAX_TEXT_WIDTH
#define MAX_TEXT_WIDTH (160)
#endif // MAX_TEXT_WIDTH

#ifndef MAX_TEXT_HEIGHT
#define MAX_TEXT_HEIGHT (90)
#endif // MAX_TEXT_HEIGHT

enum
{
  PAGE_AUTO_TEST,
  PAGE_CONJUGATE,
  PAGE_DECLINATE,
  QUIT,
  PAGE_TOTAL
};

typedef char page_row_text[MAX_TEXT_WIDTH];

typedef page_row_text page[MAX_TEXT_HEIGHT];

#endif // __PAGES_H__

//--------------------------------------

#ifdef __PAGES_VARS_IMPLEMENTATION__

page pages[PAGE_TOTAL][3] = {
    // auto-test
    {
        {"This is the statistics pages for autotests !!!",
         ";)",
         "",
         "Your score is 69/420 (nice)"},
        {"Vocab"},
        {"qt longues"},
    },
};

#endif // __PAGES_VARS_IMPLEMENTATION__