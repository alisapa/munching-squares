#include <SDL2/SDL.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>

enum mode { ZERO, FADE, NOFADE };
enum op   { XOR,  AND,  OR };

#define DEFAULT_SIZE  512
#define DEFAULT_MODE  FADE
#define DEFAULT_OP    XOR
#define DEFAULT_FADE  5
#define DEFAULT_T     0
#define DEFAULT_TSTEP 1
#define DEFAULT_DELAY 30

const char *help_str =
"Usage: %s [OPTIONS]\n"
"\n"
"Draws the \"Munching Squares\" animation.\n"
"\n"
"  -d, --delay\tDelay in ms before drawing each frame\n"
"  -f, --fade\tHow much to subtract from old values in FADE mode\n"
"  -m, --mode\tDrawing mode. One of: ZERO, FADE, NOFADE\n"
"  -o, --op\tOperation that is performed. One of: AND, OR, XOR\n"
"  -P, --paused\tPause at the beginning (un-pause with space)\n"
"  -s, --size\tSize of the window, <width>x<height>\n"
"  -t, --tstart\tStarting value for T\n"
"  -T, --tstep\tHow much to increment T in each step\n"
"  -h, --help\tShows this message and exits.\n"
"\n"
"See also: man munch(6)\n";

int min(int a, int b) {
  return a < b ? a : b;
}

void swap(unsigned int *a, unsigned int *b) {
  unsigned int tmp = *a;
  *a = *b;
  *b = tmp;
}

void strtolow(char *str) {
  for ( ; *str; ++str) *str = tolower(*str);
}

int process_uint_option(char opt, unsigned int *dest, const char *str, char **pass_endptr) {
  if (strlen(str) == 0) {
    fprintf(stderr, "Empty parameter for -%c\n", opt);
    return -1;
  }
  char *endptr;
  errno = 0;
  long tmp = strtol(str, &endptr, 0);
  if (!pass_endptr && *endptr != 0) {
    fprintf(stderr, "Invalid characters in integer parameter: %s\n", str);
    return -1;
  } else if (pass_endptr) {
    *pass_endptr = endptr;
  }
  if (errno == ERANGE || (sizeof(long) > sizeof(int) && tmp > UINT_MAX)) {
    fprintf(stderr, "Number out of range for int: %s\n", str);
    return -1;
  }
  if (tmp < 0) {
    fprintf(stderr, "Number negative: %s\n", str);
    return -1;
  }
  *dest = (unsigned int) tmp;
  return 0;
}

int init_window(SDL_Window **win, int width, int height) {
  if (SDL_Init(SDL_INIT_VIDEO)) return -1;
  SDL_Rect geom;
  if (SDL_GetDisplayUsableBounds(0, &geom)) return -1;
  *win = SDL_CreateWindow("Munching Squares", geom.x, geom.y, min(512, width),
                          min(512, height), 0);
  if (!(*win)) return -1;
  return 0;
}

int main(int argc, char **argv) {
  // Variables of the setup
  unsigned int width  = DEFAULT_SIZE;
  unsigned int height = DEFAULT_SIZE;
  enum mode mode      = DEFAULT_MODE;
  enum op op          = DEFAULT_OP;
  unsigned int fade   = DEFAULT_FADE;
  unsigned int t      = DEFAULT_T;
  unsigned int tstep  = DEFAULT_TSTEP;
  unsigned int delay  = DEFAULT_DELAY;
  int start_paused    = 0;
  SDL_Window *win     = NULL;
  SDL_Surface *s      = NULL;

  // Process options with getopt()
  int option_index = 0;
  const char *optstring = ":hd:f:m:o:Ps:t:T:";
  struct option long_options[] = {
    {"delay",  required_argument, NULL, 'd'},
    {"help",   no_argument,       NULL, 'h'},
    {"fade",   required_argument, NULL, 'f'},
    {"mode",   required_argument, NULL, 'm'},
    {"op",     required_argument, NULL, 'o'},
    {"paused", no_argument,       NULL, 'P'},
    {"size",   required_argument, NULL, 's'},
    {"tstart", required_argument, NULL, 't'},
    {"tstep",  required_argument, NULL, 'T'},
    {0,        0,                 NULL,  0 }
  };
  for (char c = getopt_long(argc, argv, optstring, long_options, &option_index);
       c != -1;
       c = getopt_long(argc, argv, optstring, long_options, &option_index)) {
    switch (c) {
      case 'h':
        printf(help_str, argv[0]);
        return 0;
      case 'd':
        if (process_uint_option('d', &delay, optarg, 0))
          return -1;
        break;
      case 'f':
        if (process_uint_option('f', &fade, optarg, 0))
          return -1;
        break;
      case 'm':
        strtolow(optarg);
        if (strcmp("zero", optarg) == 0) {
          mode = ZERO;
        } else if (strcmp("fade", optarg) == 0) {
          mode = FADE;
        } else if (strcmp("nofade", optarg) == 0) {
          mode = NOFADE;
        } else {
          fprintf(stderr, "Unknown mode \"%s\"\n", optarg);
          return -1;
        }
        break;
      case 'o':
        strtolow(optarg);
        if (strcmp("and", optarg) == 0) {
          op = AND;
        } else if (strcmp("or", optarg) == 0) {
          op = OR;
        } else if (strcmp("xor", optarg) == 0) {
          op = XOR;
        } else {
          fprintf(stderr, "Unknown operation \"%s\"\n", optarg);
          return -1;
        }
        break;
      case 'P':
        start_paused = 1;
        break;
      case 's':
        char *endptr;
        if (process_uint_option('s', &width, optarg, &endptr))
          return -1;
        if (*endptr != 'x') {
          fprintf(stderr, "Invalid characters: expected the form <width>x<height>, got: %s\n",
                  optarg);
          return -1;
        }
        if (process_uint_option('s', &height, endptr + 1, 0))
          return -1;
        break;
      case 't':
        if (process_uint_option('t', &t, optarg, 0))
          return -1;
        break;
      case 'T':
        if (process_uint_option('T', &tstep, optarg, 0))
          return -1;
        break;
      case ':':
        fprintf(stderr, "Missing argument for option -%c\n", optopt);
        return -1;
      default:
        fprintf(stderr, "Unknown option -%c\n", optopt);
        return -1;
    }
  }

  // Init window and renderer
  if (init_window(&win, width, height)) goto cleanup;

  // Init drawing surface
  SDL_Surface *scr = SDL_GetWindowSurface(win);
  s = SDL_CreateRGBSurface(0, scr->w, scr->h, 8, 0, 0, 0, 0);
  if (!s) goto cleanup;

  // Init colour palette of the drawing surface
  SDL_Color colors[256];
  for (int i = 0; i < 256; i++)
    colors[i].r = colors[i].g = colors[i].b = i;
  SDL_SetPaletteColors(s->format->palette, colors, 0, 256);

  // Actual drawing loop
  int cont = 1;
  unsigned int paused = 0;
  if (start_paused) swap(&tstep, &paused);
  SDL_Event event;
  memset(s->pixels, 0, s->h * s->pitch);
  while (cont) {
    // Draw the next iteration
    SDL_LockSurface(s);
    if (mode == ZERO) memset(s->pixels, 0, s->h * s->pitch);
    for (unsigned int x = 0; x < (unsigned int) s->w; x++) {
      if (mode == FADE) {
        for (unsigned int ys = 0; ys < (unsigned int) s->h; ys++) {
          if (((uint8_t*) s->pixels)[ys * s->pitch + x] != 0)
            ((uint8_t*) s->pixels)[ys * s->pitch + x] -= fade;
        }
      }
      unsigned int y = (
        op == XOR ? (x ^ t) :
        op == AND ? (x & t) :
        op == OR  ? (x | t) : 0 ) % s->h;
      ((uint8_t*) s->pixels)[y * s->pitch + x] = 255;
    }
    SDL_UnlockSurface(s);
    SDL_BlitSurface(s, NULL, SDL_GetWindowSurface(win), NULL);
    SDL_UpdateWindowSurface(win);

    t += tstep;
    
    // Event stuff
    SDL_PollEvent(&event);
    switch (event.type) {
      case SDL_QUIT:
        cont = 0;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE ||
            event.key.keysym.sym == SDLK_q) {
          cont = 0;
        } else if (event.key.keysym.sym == SDLK_SPACE) {
          swap(&tstep, &paused);
        } else if (event.key.keysym.sym == SDLK_RETURN) {
          // Freeze until Enter pressed again
          do {
            SDL_Delay(delay);
            SDL_PollEvent(&event);
          } while (event.type != SDL_KEYDOWN || event.key.keysym.sym != SDLK_RETURN);
        } else if (event.key.keysym.sym == SDLK_LEFT) {
          if (!paused)
            tstep--;
          else
            paused--;
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
          if (!paused)
            tstep++;
          else
            paused++;
        }
        break;
      default:
        break;
    }

    SDL_Delay(delay);
  }

  SDL_FreeSurface(s);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
 
 cleanup:
  fprintf(stderr, "SDL error: %s\n", SDL_GetError());
  if (s) SDL_FreeSurface(s);
  if (win) SDL_DestroyWindow(win);
  SDL_Quit();
  return -1;
}
