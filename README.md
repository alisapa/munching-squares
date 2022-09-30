# Munching Squares

This program draws the "munching squares" animation, which is a plot of Y = X XOR T for consecutive values of T.

## Options

There are several options for producing different animations. See the the manpage `munch(6)` for a full documentation; it can be read without installation by invoking `man man/munch.6`.

Invoke `munch --help` for a short summary of options.
```
Usage: ./munch [OPTIONS]

Draws the "Munching Squares" animation.

  -d, --delay	Delay in ms before drawing each frame
  -f, --fade	How much to subtract from old values in FADE mode
  -m, --mode	Drawing mode. One of: ZERO, FADE, NOFADE
  -o, --op	Operation that is performed. One of: AND, OR, XOR
  -P, --paused	Paused at the beginning, un-pause with space
  -s, --size	Size of the window, <width>x<height>
  -t		Starting value for T
  -T, --tstep	How much to increment T in each step
  -h, --help	Shows this message and exits.

See also: man munch(6)
```

## Installation

Dependencies:
- [SDL2](https://www.libsdl.org)

Compiling:
```bash
make
```

Global installation:
```bash
make install
```

Local installation (by setting different `INSTALL_PREFIX`):
```bash
make install INSTALL_PREFIX=~/.local
```

If doing a local installation, make sure that the relevant directories have been added to `PATH` and `MANPATH`.

Removal:
```bash
make uninstall
```

Note that if `INSTALL_PREFIX` set during installation, it must be also be set, to the same value, during removal.

## See also
- [DEC PDP-7 w/ Type 340 display running Munching Squares and Spirograph](https://www.youtube.com/watch?v=V4oRHv-Svwc) --- its phosphor screen produces a more elaborate fading effect than this program
- [http://www.inwap.com/pdp10/hbaker/hakmem/hacks.html#item146](First mention and description of Munching Squares, from the HAKMEM document)
