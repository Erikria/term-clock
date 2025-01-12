/*
 *      TERM-CLOCK headers file.
 *      Copyright © 2023-2026 term-clock contributors
 *      Copyright © 2009-2013 tty-clock contributors
 *      Copyright © 2008-2009 Martin Duquesnoy <xorg62@gmail.com>
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of the  nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TERM_CLOCK_H_INCLUDED
#define TERM_CLOCK_H_INCLUDED

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* Macro */
#define NORMFRAMEW 35
#define SECFRAMEW 54
#define DATEWINH 3
#define AMSIGN " [AM]"
#define PMSIGN " [PM]"

/* Global TermClock struct */
struct TermClock {
  /* while() boolean */
  bool running;

  /* terminal variables */
  SCREEN *term_screen;
  char *term;
  int bg;

  /* Running option */
  struct {
    bool second;
    bool screensaver;
    bool twelve;
    bool center;
    bool rebound;
    bool date;
    bool utc;
    bool box;
    bool noquit;
    char format[100];
    int color;
    bool bold;
    long delay;
    bool blink;
    long nsdelay;
  } option;

  /* Clock geometry */
  struct {
    int x, y, w, h;
    /* For rebound use (see clock_rebound())*/
    int a, b;
  } geo;

  /* Date content ([2] = number by number) */
  struct {
    unsigned int hour[2];
    unsigned int minute[2];
    unsigned int second[2];
    char datestr[256];
    char old_datestr[256];
  } date;

  /* time.h utils */
  struct tm *tm;
  time_t lt;

  /* Clock member */
  char *meridiem;
  WINDOW *framewin;
  WINDOW *datewin;
};

/* Prototypes */
void init(void);
void signal_handler(int signal);
void update_hour(void);
void draw_number(int n, int x, int y);
void draw_clock(void);
void clock_move(int x, int y, int w, int h);
void set_second(void);
void set_center(bool b);
void set_box(bool b);
void key_event(void);

/* Number matrix */
const bool number[][15] = {
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1}, /* 0 */
    {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1}, /* 1 */
    {1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1}, /* 2 */
    {1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1}, /* 3 */
    {1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1}, /* 4 */
    {1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1}, /* 5 */
    {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1}, /* 6 */
    {1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1}, /* 7 */
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1}, /* 8 */
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1}, /* 9 */
};

#endif /* TTYCLOCK_H_INCLUDED */
