/*
 *      TERM-CLOCK Main file.
 *      Copyright © 2023-2026 term-clock contributors
 *      Copyright © 2009-2018 tty-clock contributors
 *      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
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

#include "clock.h"

/* Global variable */
struct TermClock term_clock;

void init(void) {
  struct sigaction sig;
  setlocale(LC_TIME, "");

  term_clock.bg = COLOR_BLACK;

  /* Init ncurses */
  if (term_clock.term) {
    FILE *f_term = fopen(term_clock.term, "r+");
    if (!f_term) {
      fprintf(stderr, "term-clock: error: '%s' couldn't be opened: %s.\n",
              term_clock.term, strerror(errno));
      exit(EXIT_FAILURE);
    }
    term_clock.term_screen = newterm(NULL, f_term, f_term);
    assert(term_clock.term_screen != NULL);
    set_term(term_clock.term_screen);
  } else
    initscr();

  cbreak();
  noecho();
  keypad(stdscr, true);
  start_color();
  curs_set(false);
  clear();

  /* Init default terminal color */
  if (use_default_colors() == OK)
    term_clock.bg = -1;

  /* Init color pair */
  init_pair(0, term_clock.bg, term_clock.bg);
  init_pair(1, term_clock.bg, term_clock.option.color);
  init_pair(2, term_clock.option.color, term_clock.bg);
  //     init_pair(0, term_clock.bg, term_clock.bg);
  //     init_pair(1, term_clock.bg, term_clock.option.color);
  //     init_pair(2, term_clock.option.color, term_clock.bg);
  refresh();

  /* Init signal handler */
  sig.sa_handler = signal_handler;
  sig.sa_flags = 0;
  sigaction(SIGTERM, &sig, NULL);
  sigaction(SIGINT, &sig, NULL);
  sigaction(SIGSEGV, &sig, NULL);

  /* Init global struct */
  term_clock.running = true;
  if (!term_clock.geo.x)
    term_clock.geo.x = 0;
  if (!term_clock.geo.y)
    term_clock.geo.y = 0;
  if (!term_clock.geo.a)
    term_clock.geo.a = 1;
  if (!term_clock.geo.b)
    term_clock.geo.b = 1;
  term_clock.geo.w = (term_clock.option.second) ? SECFRAMEW : NORMFRAMEW;
  term_clock.geo.h = 7;
  term_clock.tm = localtime(&(term_clock.lt));
  if (term_clock.option.utc) {
    term_clock.tm = gmtime(&(term_clock.lt));
  }
  term_clock.lt = time(NULL);
  update_hour();

  /* Create clock win */
  term_clock.framewin =
      newwin(term_clock.geo.h, term_clock.geo.w, term_clock.geo.x, term_clock.geo.y);
  if (term_clock.option.box) {
    box(term_clock.framewin, 0, 0);
  }

  if (term_clock.option.bold) {
    wattron(term_clock.framewin, A_BLINK);
  }

  /* Create the date win */
  term_clock.datewin = newwin(DATEWINH, strlen(term_clock.date.datestr) + 2,
                            term_clock.geo.x + term_clock.geo.h - 1,
                            term_clock.geo.y + (term_clock.geo.w / 2) -
                                (strlen(term_clock.date.datestr) / 2) - 1);
  if (term_clock.option.box && term_clock.option.date) {
    box(term_clock.datewin, 0, 0);
  }
  clearok(term_clock.datewin, true);

  set_center(term_clock.option.center);

  nodelay(stdscr, true);

  if (term_clock.option.date) {
    wrefresh(term_clock.datewin);
  }

  wrefresh(term_clock.framewin);

  return;
}

void signal_handler(int signal) {
  switch (signal) {
  case SIGINT:
  case SIGTERM:
    term_clock.running = false;
    break;
    /* Segmentation fault signal */
  case SIGSEGV:
    endwin();
    fprintf(stderr, "Segmentation fault.\n");
    exit(EXIT_FAILURE);
    break;
  }

  return;
}

void cleanup(void) {
  if (term_clock.term_screen)
    delscreen(term_clock.term_screen);

  free(term_clock.term);
}

void update_hour(void) {
  int ihour;
  char tmpstr[128];

  term_clock.lt = time(NULL);
  term_clock.tm = localtime(&(term_clock.lt));
  if (term_clock.option.utc) {
    term_clock.tm = gmtime(&(term_clock.lt));
  }

  ihour = term_clock.tm->tm_hour;

  if (term_clock.option.twelve)
    term_clock.meridiem = ((ihour >= 12) ? PMSIGN : AMSIGN);
  else
    term_clock.meridiem = "\0";

  /* Manage hour for twelve mode */
  ihour = ((term_clock.option.twelve && ihour > 12) ? (ihour - 12) : ihour);
  ihour = ((term_clock.option.twelve && !ihour) ? 12 : ihour);

  /* Set hour */
  term_clock.date.hour[0] = ihour / 10;
  term_clock.date.hour[1] = ihour % 10;

  /* Set minutes */
  term_clock.date.minute[0] = term_clock.tm->tm_min / 10;
  term_clock.date.minute[1] = term_clock.tm->tm_min % 10;

  /* Set date string */
  strcpy(term_clock.date.old_datestr, term_clock.date.datestr);
  strftime(tmpstr, sizeof(tmpstr), term_clock.option.format, term_clock.tm);
  sprintf(term_clock.date.datestr, "%s%s", tmpstr, term_clock.meridiem);

  /* Set seconds */
  term_clock.date.second[0] = term_clock.tm->tm_sec / 10;
  term_clock.date.second[1] = term_clock.tm->tm_sec % 10;

  return;
}

void draw_number(int n, int x, int y) {
  int i, sy = y;

  for (i = 0; i < 30; ++i, ++sy) {
    if (sy == y + 6) {
      sy = y;
      ++x;
    }

    if (term_clock.option.bold)
      wattron(term_clock.framewin, A_BLINK);
    else
      wattroff(term_clock.framewin, A_BLINK);

    wbkgdset(term_clock.framewin, COLOR_PAIR(number[n][i / 2]));
    mvwaddch(term_clock.framewin, x, sy, ' ');
  }
  wrefresh(term_clock.framewin);

  return;
}

void draw_clock(void) {
  if (term_clock.option.date && !term_clock.option.rebound &&
      strcmp(term_clock.date.datestr, term_clock.date.old_datestr) != 0) {
    clock_move(term_clock.geo.x, term_clock.geo.y, term_clock.geo.w, term_clock.geo.h);
  }

  /* Draw hour numbers */
  draw_number(term_clock.date.hour[0], 1, 1);
  draw_number(term_clock.date.hour[1], 1, 8);
  chtype dotcolor = COLOR_PAIR(1);
  if (term_clock.option.blink && time(NULL) % 2 == 0)
    dotcolor = COLOR_PAIR(2);

  /* 2 dot for number separation */
  wbkgdset(term_clock.framewin, dotcolor);
  mvwaddstr(term_clock.framewin, 2, 16, "  ");
  mvwaddstr(term_clock.framewin, 4, 16, "  ");

  /* Draw minute numbers */
  draw_number(term_clock.date.minute[0], 1, 20);
  draw_number(term_clock.date.minute[1], 1, 27);

  /* Draw the date */
  if (term_clock.option.bold)
    wattron(term_clock.datewin, A_BOLD);
  else
    wattroff(term_clock.datewin, A_BOLD);

  if (term_clock.option.date) {
    wbkgdset(term_clock.datewin, (COLOR_PAIR(2)));
    mvwprintw(term_clock.datewin, (DATEWINH / 2), 1, "%s", term_clock.date.datestr);
    wrefresh(term_clock.datewin);
  }

  /* Draw second if the option is enabled */
  if (term_clock.option.second) {
    /* Again 2 dot for number separation */
    wbkgdset(term_clock.framewin, dotcolor);
    mvwaddstr(term_clock.framewin, 2, NORMFRAMEW, "  ");
    mvwaddstr(term_clock.framewin, 4, NORMFRAMEW, "  ");

    /* Draw second numbers */
    draw_number(term_clock.date.second[0], 1, 39);
    draw_number(term_clock.date.second[1], 1, 46);
  }

  return;
}

void clock_move(int x, int y, int w, int h) {

  /* Erase border for a clean move */
  wbkgdset(term_clock.framewin, COLOR_PAIR(0));
  wborder(term_clock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  werase(term_clock.framewin);
  wrefresh(term_clock.framewin);

  if (term_clock.option.date) {
    wbkgdset(term_clock.datewin, COLOR_PAIR(0));
    wborder(term_clock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(term_clock.datewin);
    wrefresh(term_clock.datewin);
  }

  /* Frame win move */
  mvwin(term_clock.framewin, (term_clock.geo.x = x), (term_clock.geo.y = y));
  wresize(term_clock.framewin, (term_clock.geo.h = h), (term_clock.geo.w = w));

  /* Date win move */
  if (term_clock.option.date) {
    mvwin(term_clock.datewin, term_clock.geo.x + term_clock.geo.h - 1,
          term_clock.geo.y + (term_clock.geo.w / 2) -
              (strlen(term_clock.date.datestr) / 2) - 1);
    wresize(term_clock.datewin, DATEWINH, strlen(term_clock.date.datestr) + 2);

    if (term_clock.option.box) {
      box(term_clock.datewin, 0, 0);
    }
  }

  if (term_clock.option.box) {
    box(term_clock.framewin, 0, 0);
  }

  wrefresh(term_clock.framewin);
  wrefresh(term_clock.datewin);
  return;
}

/* Useless but fun :) */
void clock_rebound(void) {
  if (!term_clock.option.rebound)
    return;

  if (term_clock.geo.x < 1)
    term_clock.geo.a = 1;
  if (term_clock.geo.x > (LINES - term_clock.geo.h - DATEWINH))
    term_clock.geo.a = -1;
  if (term_clock.geo.y < 1)
    term_clock.geo.b = 1;
  if (term_clock.geo.y > (COLS - term_clock.geo.w - 1))
    term_clock.geo.b = -1;

  clock_move(term_clock.geo.x + term_clock.geo.a, term_clock.geo.y + term_clock.geo.b,
             term_clock.geo.w, term_clock.geo.h);

  return;
}

void set_second(void) {
  int new_w =
      (((term_clock.option.second = !term_clock.option.second)) ? SECFRAMEW
                                                            : NORMFRAMEW);
  int y_adj;

  for (y_adj = 0; (term_clock.geo.y - y_adj) > (COLS - new_w - 1); ++y_adj)
    ;

  clock_move(term_clock.geo.x, (term_clock.geo.y - y_adj), new_w, term_clock.geo.h);

  set_center(term_clock.option.center);

  return;
}

void set_center(bool b) {
  if ((term_clock.option.center = b)) {
    term_clock.option.rebound = false;

    clock_move((LINES / 2 - (term_clock.geo.h / 2)),
               (COLS / 2 - (term_clock.geo.w / 2)), term_clock.geo.w,
               term_clock.geo.h);
  }

  return;
}

void set_box(bool b) {
  term_clock.option.box = b;

  wbkgdset(term_clock.framewin, COLOR_PAIR(0));
  wbkgdset(term_clock.datewin, COLOR_PAIR(0));

  if (term_clock.option.box) {
    wbkgdset(term_clock.framewin, COLOR_PAIR(0));
    wbkgdset(term_clock.datewin, COLOR_PAIR(0));
    box(term_clock.framewin, 0, 0);
    box(term_clock.datewin, 0, 0);
  } else {
    wborder(term_clock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wborder(term_clock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  }

  wrefresh(term_clock.datewin);
  wrefresh(term_clock.framewin);
}

void key_event(void) {
  int i, c;

  struct timespec length = {term_clock.option.delay, term_clock.option.nsdelay};

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(STDIN_FILENO, &rfds);

  if (term_clock.option.screensaver) {
    c = wgetch(stdscr);
    if (c != ERR && term_clock.option.noquit == false) {
      term_clock.running = false;
    } else {
      nanosleep(&length, NULL);
      for (i = 0; i < 8; ++i)
        if (c == (i + '0')) {
          term_clock.option.color = i;
          init_pair(1, term_clock.bg, i);
          init_pair(2, i, term_clock.bg);
        }
    }
    return;
  }

  switch (c = wgetch(stdscr)) {
  case KEY_RESIZE:
    endwin();
    init();
    break;

  case KEY_UP:
  case 'k':
  case 'K':
    if (term_clock.geo.x >= 1 && !term_clock.option.center)
      clock_move(term_clock.geo.x - 1, term_clock.geo.y, term_clock.geo.w,
                 term_clock.geo.h);
    break;

  case KEY_DOWN:
  case 'j':
  case 'J':
    if (term_clock.geo.x <= (LINES - term_clock.geo.h - DATEWINH) &&
        !term_clock.option.center)
      clock_move(term_clock.geo.x + 1, term_clock.geo.y, term_clock.geo.w,
                 term_clock.geo.h);
    break;

  case KEY_LEFT:
  case 'h':
  case 'H':
    if (term_clock.geo.y >= 1 && !term_clock.option.center)
      clock_move(term_clock.geo.x, term_clock.geo.y - 1, term_clock.geo.w,
                 term_clock.geo.h);
    break;

  case KEY_RIGHT:
  case 'l':
  case 'L':
    if (term_clock.geo.y <= (COLS - term_clock.geo.w - 1) &&
        !term_clock.option.center)
      clock_move(term_clock.geo.x, term_clock.geo.y + 1, term_clock.geo.w,
                 term_clock.geo.h);
    break;

  case 'q':
  case 'Q':
    if (term_clock.option.noquit == false)
      term_clock.running = false;
    break;

  case 's':
  case 'S':
    set_second();
    break;

  case 't':
  case 'T':
    term_clock.option.twelve = !term_clock.option.twelve;
    /* Set the new term_clock.date.datestr to resize date window */
    update_hour();
    clock_move(term_clock.geo.x, term_clock.geo.y, term_clock.geo.w, term_clock.geo.h);
    break;

  case 'c':
  case 'C':
    set_center(!term_clock.option.center);
    break;

  case 'b':
  case 'B':
    term_clock.option.bold = !term_clock.option.bold;
    break;

  case 'r':
  case 'R':
    term_clock.option.rebound = !term_clock.option.rebound;
    if (term_clock.option.rebound && term_clock.option.center)
      term_clock.option.center = false;
    break;

  case 'x':
  case 'X':
    set_box(!term_clock.option.box);
    break;

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
    i = c - '0';
    term_clock.option.color = i;
    init_pair(1, term_clock.bg, i);
    init_pair(2, i, term_clock.bg);
    break;

  default:
    pselect(1, &rfds, NULL, NULL, &length, NULL);
  }

  return;
}

int main(int argc, char **argv) {
  int c;

  /* Alloc term_clock */
  memset(&term_clock, 0, sizeof(struct TermClock));

  term_clock.option.date = true;

  /* Default date format */
  strncpy(term_clock.option.format, "%F", sizeof(term_clock.option.format));
  /* Default color */
  term_clock.option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */
  /* Default delay */
  term_clock.option.delay = 1;   /* 1FPS */
  term_clock.option.nsdelay = 0; /* -0FPS */
  term_clock.option.blink = false;

  atexit(cleanup);

  while ((c = getopt(argc, argv, "iuvsScbtrhBxnDC:f:d:T:a:")) != -1) {
    switch (c) {
    case 'h':
    default:
      printf(
          "Usage: term-clock [-iuvsScbtrahDBxn] [-C [0-7]] [-f format] [-d "
          "delay] [-a nsdelay] [-T term] \n"
          "    -s            Show seconds                                   \n"
          "    -S            Screensaver mode                               \n"
          "    -x            Show box                                       \n"
          "    -c            Set the clock at the center of the terminal    \n"
          "    -C [0-7]      Set the clock color                            \n"
          "    -b            Use bold colors                                \n"
          "    -t            Set the hour in 12h format                     \n"
          "    -u            Use UTC time                                   \n"
          "    -T term       Display the clock on the specified terminal    \n"
          "    -r            Do rebound the clock                           \n"
          "    -f format     Set the date format                            \n"
          "    -n            Do not quit on keypress                         \n"
          "    -v            Show term-clock version                         \n"
          "    -i            Show some info about term-clock                 \n"
          "    -h            Show this page                                 \n"
          "    -D            Hide date                                      \n"
          "    -B            Enable blinking colon                          \n"
          "    -d delay      Set the delay between two redraws of the clock. "
          "Default 1s. \n"
          "    -a nsdelay    Additional delay between two redraws in "
          "nanoseconds. Default 0ns.\n");
      exit(EXIT_SUCCESS);
      break;
    case 'i':
      puts("Term-Clock by Erikria, forked from <https://github.com/xorg62/tty-clock>.");
      exit(EXIT_SUCCESS);
      break;
    case 'u':
      term_clock.option.utc = true;
      break;
    case 'v':
      puts("Term-Clock 20231105");
      exit(EXIT_SUCCESS);
      break;
    case 's':
      term_clock.option.second = true;
      break;
    case 'S':
      term_clock.option.screensaver = true;
      break;
    case 'c':
      term_clock.option.center = true;
      break;
    case 'b':
      term_clock.option.bold = true;
      break;
    case 'C':
      if (atoi(optarg) >= 0 && atoi(optarg) < 8)
        term_clock.option.color = atoi(optarg);
      break;
    case 't':
      term_clock.option.twelve = true;
      break;
    case 'r':
      term_clock.option.rebound = true;
      break;
    case 'f':
      strncpy(term_clock.option.format, optarg, 100);
      break;
    case 'd':
      if (atol(optarg) >= 0 && atol(optarg) < 100)
        term_clock.option.delay = atol(optarg);
      break;
    case 'D':
      term_clock.option.date = false;
      break;
    case 'B':
      term_clock.option.blink = true;
      break;
    case 'a':
      if (atol(optarg) >= 0 && atol(optarg) < 1000000000)
        term_clock.option.nsdelay = atol(optarg);
      break;
    case 'x':
      term_clock.option.box = true;
      break;
    case 'T': {
      struct stat sbuf;
      if (stat(optarg, &sbuf) == -1) {
        fprintf(stderr, "term-clock: error: couldn't stat '%s': %s.\n", optarg,
                strerror(errno));
        exit(EXIT_FAILURE);
      } else if (!S_ISCHR(sbuf.st_mode)) {
        fprintf(
            stderr,
            "term-clock: error: '%s' doesn't appear to be a character device.\n",
            optarg);
        exit(EXIT_FAILURE);
      } else {
        free(term_clock.term);
        term_clock.term = strdup(optarg);
      }
    } break;
    case 'n':
      term_clock.option.noquit = true;
      break;
    }
  }

  init();
  attron(A_BLINK);
  while (term_clock.running) {
    clock_rebound();
    update_hour();
    draw_clock();
    key_event();
  }

  endwin();

  return 0;
}
