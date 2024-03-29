About this document:
Last modified by Alex Smith, 2015-02-02

Copyright © 2013, 2014, 2015 Alex Smith.

This documentation for the uncursed rendering library is provided under the
same terms as libuncursed itself: it may be distributed under either of the
following licenses:

  * the NetHack General Public License
  * the GNU General Public License v2 or later

If you obtained this documentation as part of NetHack 4, you can find these
licenses in the files libnethack/dat/license and libnethack/dat/gpl
respectively.


The uncursed rendering library
==============================

uncursed is a library intended primarily for roguelikes and similarly
graphically simple screen-oriented applications which are designed to be
played in a terminal.  It has three main purposes:

  * If players are playing in a terminal, to aim for correct, consistent
    rendering on a wide range of terminal types (even if its output is
    recorded and later played back on a different terminal);
  * For players who cannot, or do not wish to, play in a terminal (perhaps
    because their platform does not have a high-quality terminal emulator), to
    provide a similar play experience via other means (e.g. via simulating a
    terminal);
  * To provide support for optional, advanced features (such as tiles play or
    mouse support) in a way that requires as little extra effort as possible
    by the developers of the game that uses libuncursed.

Some things that are *not* goals of uncursed:

  * uncursed aims for compatibility with commonly used operating systems and
    platforms, but not for perfect compatibility with every terminal ever
    created. Thus, in applications where you want to communicate with an
    actual, physical VT100, for instance, a library like ncurses will be a
    better fit; libuncursed would try to send control codes too fast for the
    terminal, whereas libncurses will intentionally insert delays.
  * uncursed will not aim to produce substitute codes on primitive terminals
    in order to approximate rendering that would exist on more complex
    terminals. For instance, on terminals that do not understand background
    colors, it will not attempt to approximate the background color using
    inverse colors, like might happen with a terminal-capability-aware
    application.
  * uncursed will not use, or provide access to, terminal features that are
    not widely available; if a feature does not exist on a wide range of
    terminals, it's likely to be inexpressible in libuncursed.

This guide documents the use of libuncursed for users of programs that use it,
for developers who want to write a game using libuncursed (or to convert an
existing program to use it), and for developers who want to write new plugins
for libuncursed.


Using a libuncursed program
===========================

uncursed has a plugin-based architecture.  Thus, the main customization option
for a user (apart from any options that the program you are using might give
you) is as to which plugins you load.

The main choice is as to which plugin will be responsible for the main
interface of the application.  You can specify this in several ways:

  * On the command line, using `--interface plugin` at the start of the
    command you use to invoke the application (e.g. `--interface tty` if you
    want to use the `tty` interface);
  * Via creating a symbolic link to the application (or renaming the
    application) to end with a hyphen and the name of a plugin (e.g. if you
    create a symbolic link to `program` as `program-sdl`), running the program
    via that symbolic link will run it using the `sdl` interface;
  * Using a default depending on your current platform and the way that
    libuncursed and the application are configured, otherwise.

Here are the main interface plugins available:

tty
---

The `tty` interface is available for UNIX platforms (such as Mac OS X) and
UNIX-like platforms (such as Linux), and produces output suitable for use in a
modern terminal that uses VT100-inspired control codes.  (The terminal does
not necessarily have to be running on the same operating system as libuncursed
itself; roguelikes are commonly played over ssh or telnet, and the `tty`
interface is specifically supported in this sort of configuration.)

This interface is tested and known to work on the following terminals:

  * Terminals built into operating systems:
      * The terminal provided by `fbcon` (the Linux Framebuffer Console
        Support), which is typically compiled into the Linux kernel and used
        for the text consoles if no graphical interface is loaded;
      * The terminal used to render text by DOSBox (the terminal used by DOS
        itself probably also works if ANSI.SYS is loaded, but this is very
        hard to test because it would require finding an old DOS
        implementation with a working network stack and telnet program);
  * Terminal emulation software:
      * `xterm` (traditionally used as the terminal emulator under X11, the
        most commonly used graphical interface under Linux and BSD);
      * `gnome-terminal` (typically used as the terminal emulator under
        Gnome-based environments such as Gnome, MATE, and Ubuntu Unity);
      * `konsole` (typically used as the terminal emulator under KDE and
        KDE-based environments);
      * `lxterminal` (typically used as the terminal under LXDE);
      * `PuTTY` (the most commonly used terminal emulator for ssh and telnet
        connections under Windows);
      * `urxvt` (the Unicode-aware version of `rxvt`, which was once the main
        competitor to `xterm`);
  * Terminal multiplexers:
      * `screen`, `tmux` (both typically used in order to keep a program
        running while disconnected from the computer it runs on, or to allow
        switching between multiple screen-oriented programs or to allow
        multiple people to drive the same screen-oriented program);
  * Terminal recording renderers:
      * `termplay` (commonly used to play back terminal recordings on
        Windows, and which also works on other systems);
      * `jettyplay` (a portable Java-based GUI terminal recording renderer);
      * and any terminal recording player that uses the terminal it runs in
        for rendering (such as `ttyplay`), if that terminal is supported by
        the uncursed `tty` interface.

The most notable terminals on which this interface does *not* work are:

  * `rxvt`, which relies on its font files for encoding support in a way that
    typically produces garbled output of non-ASCII characters when this
    interface is used (if you can, use `urxvt` instead, which does not have
    this problem);
  * `ipbt`, whose problem is that it can parse codes for 16-color-aware
    terminals but subsequently does not know how to render them (this may be a
    bug in `ipbt`);
  * any physical terminal that requires the application using it to insert
    delays into the data it sends;
  * and the console that ships with Windows 2000 and later (which does not
    parse terminal control codes at all; if you really want to use this
    console for some reason, use the `wincon` interface plugin).

Unlike with ncurses, the uncursed `tty` interface plugin does not require any
help from environment variables such as `TERM`, or support files such as
terminal capabilities databases, to function correctly; instead, it aims to
produce codes that will be recognised by as many terminals as possible (if
necessary, via concatenating codes for different terminals in a way that will
cause all the supported terminals to parse the bits intended for them, and
ignore the bits intended for other terminals).  This means that the terminal
will render the data correctly even if `TERM` is set incorrectly (a common
problem over ssh; although ssh sends the value of `TERM` correctly, it can
often has a different meaning on the source and target machines if the
operating systems differ), and even if the terminal capabilities database is
wrong or insufficient (which is a common cause of dark gray rendering
incorrectly on terminals; some terminals will render dark gray incorrectly on
ncurses but correctly via libuncursed).

This interface supports most of the features of libuncursed: 8 foreground and
8 background colors, bold, underlining (if the terminal supports it), cursor
movement / showing / hiding, resizing, and (if the terminal supports it) mouse
support; it will attempt to use either code page 437 ("IBMgraphics") or UTF-8
for handling non-ASCII characters, depending on what the terminal supports.
It cannot (for obvious reasons) handle features like tiles support that
require a GUI, and will just ignore any information about fonts or tiles that
the application sends it.

wincon
------

The `wincon` interface plugin is similar to the `tty` interface plugin in
terms of what it does: it renders the program in a terminal emulator.
However, instead of using VT100-like control codes, it uses the Windows
console API (introduced in Windows 2000, although this interface has only
currently been tested on Windows 7), and thus renders into the Windows
console, rather than a terminal emulator, working only for native play
(i.e. not over telnet or ssh) and only on Windows.

The main thing to bear in mind with this interface is that the Windows console
has exceptionally bad performance, as terminal emulators go.  The `wincon`
interface plugin attempts to alleviate this problem somewhat via favouring
speed over correctness (e.g. it will delay redraws requested by the underlying
program until it requests a key or a timed delay), but it will still be
noticeably slower than other interfaces.

One other issue is that the way that the Windows console handles resizing is
bizarre: a window acts as a scrollable viewport into a larger underlying
terminal, and both the window and underlying terminal can be resized.  (This
is why the Windows console cannot by default be resized wider, only taller,
and why it always has a constant amount of scrollback even if there's nothing
to scroll back to.)  The `wincon` interface will, by default, set the
underlying terminal to the size of the screen, and render only within the area
shown in the viewport, meaning that the terminal can effectively be resized
via dragging the window borders, just like on the other interfaces.  However,
you need to press a key after resizing, because Windows does not notify
console programs of changes to the viewport size (only to the underlying
terminal size), and scrollbars will be constantly visible (but cause rendering
issues if not scrolled to the top left).  If you resize the underlying
terminal while the uncursed-using program is open, the `wincon` interface will
instead start acting like Windows programs are "meant" to act, rendering on
the entire underlying terminal and ignoring the viewport size, meaning that
you can scroll around within the view or have a view larger than the screen,
but will not be able to resize the terminal via dragging the window borders
until you restart the program.

sdl
---

The `sdl` interface plugin uses version 2 of the Simple DirectMedia Layer
library (SDL) to render a simulated terminal in a portable way across a very
wide range of systems.  It aims to simulate, as far as possible, the
experience of the `tty` interface plugin running in a terminal emulator
(although it does not bother with a conversion to VT100 control codes and
back, because this would be invisible to a user anyway).  Thus, it gives a
grid-based view with foreground and background colors and a cursor, just like
a real terminal emulator does.

In order to be able to draw characters, this interface needs a font file; the
program that loads libuncursed is responsible for telling libuncursed where it
is.  If no font file is available, the `sdl` interface plugin will not be able
to render characters, and will render everything as a solid block of color.
(If you see this symptom when running a program with this interface plugin,
try running the program from a terminal; it will send any error messages about
failure to find fonts to that terminal.)

This plugin can also handle mouse input, and extended graphical capabilities
such as tiles, if requested to do so by the program that uses it.


Writing a program that uses libuncursed
=======================================

libuncursed's API is designed to be reminisent of that of the older terminal
rendering libraries ncurses and pdcurses (which are in turn based on the
API of curses).  It does *not* aim to be a curses implementation, nor 100%
compatible with the curses standard; however, it tries to be close enough that
porting existing curses programs to uncursed does not require major
changes.

uncursed is currently only compatible with C programs (although other
languages can use its features via using whatever methods they would typically
use to interface with a library written in C).  To use libuncursed's
capabilities within a program, you will need to link that program with the
appropriate shared library, import library or static library for the platform
(`libuncursed.so`, `libuncursed.dll.a`, or `libuncursed.a`); typically, you
would just place `-luncursed` on your linker or compiler driver's command line
and let it figure out the details. You will also need to include the header
file, `#include <uncursed.h>`, in every C source file that mentions a
libuncursed function or variable.  (This header includes `<stddef.h>`,
`<stdarg.h>`, and `<wchar.h>`; it also defines a number of private macros,
typedefs, and the like, all of which start with `uncursed_` or `UNCURSED_`, in
addition to the functions, typedefs, and macros that are part of the public
API.)

There is a standard naming convention for API functions (that is also used by
curses):

  * Functions with no prefix operate on `stdscr` and the current cursor
    location;
  * Functions with a `w` prefix are identical to functions without the prefix,
    except that they take an extra `WINDOW *` argument as their first
    argument, which specifies which window to operate on;
  * Functions with a `mv` prefix are identical to functions without the
    prefix, except that they take two extra `int`s as their first and second
    arguments (which represent a y and x coordinate in that order), and move
    the cursor to the specified location before doing anything else;
  * Functions with a `mvw` prefix combine the above cases; they operate on a
    window specified as the first argument, and move the cursor to the
    coordinates specified by their second and third arguments before
    performing the requested operation.

Not all functions allow all prefixes; where prefixes are allowed, they will be
mentioned in the API reference below.

Typedefs
--------

`wchar_t` is a standard C94 definition (that is included from your system
header files by libuncursed); there is also `wint_t` (which can fit either a
`wchar_t` or an `int`, whichever is larger).  The size of a `wchar_t` depends
on your platform, and it represents one Unicode codepoint (libuncursed works
using Unicode internally).  Note that on some operating systems, characters
outside the Basic Multilingual Plane do not fit in a `wchar_t`, and thus are
not supported by libuncursed; this is typically not a problem, as those
operating systems would be unable to render the characters in question anyway.
Whenever libuncursed sees a plain `char`, it's interpreted as being encoded in
code page 437 (a superset of ASCII, and the one most generally accepted by
terminal emulators); although you should confine yourself to code page 437 if
you can for portability reasons, we recommend using the Unicode versions of
functions where possible so as to have the best possible Unicode support on
systems and terminals where it exists.

`WINDOW` (a typedef for `struct WINDOW`) represents an uncursed window: these
are rectangular buffers stored in memory, that contain characters and
attributes.  The fields of the structure should only be accessed via
accessors, not directly.  A window has a size, some contents, and a suggested
screen location (as well as several settings that can be toggled, and a
current attribute and cursor location); however, there is no particular reason
why a window should necessarily fit within the screen, or indeed ever be drawn
to the screen, so they can be used to store information independently of their
screen coordinates.  It is also possible to create multiple windows that are
views into the same memory buffer; a "parent" window that is the buffer
itself, and "child" windows that look at rectangular regions of it.

An `attr_t` is a numeric type that represents a text attribute: everything
about one character on screen (color, underlining, etc.) apart from the
codepoint that's being rendered itself.  For curses compatibility, it is
possible to bitwise OR an `attr_t` with a `char` to form a `chtype`, a
character with a codepoint and attributes; however, this is unsatisfactory as
it only works on code page 437 characters, rather than Unicode as a whole.
Instead, we recommend using the `cchar_t` type (a typedef for a struct), which
can be packed and unpacked from an `attr_t` and some `wchar_t`s using the
`setcchar` and `getcchar` functions (as in ncursesw, the Unicode version of
ncurses).

Finally, there is the `uncursed_color` type, which is a numerical type with a
sufficient range to store color codes and color pair codes, and the
`uncursed_bool` type, with enough range to store 0 and 1 (and possibly other
values).  Both these values can be safely stored in an `int`.

Global variables
----------------

Perhaps unfortunately, curses has a history of using global variables as one
method of communication between the rendering library and the program using
it.

`WINDOW *stdscr` is the default window created when uncursed is initialised.
It is always the same size as the screen of the terminal that uncursed renders
onto (which is likely to be a window in a GUI in practice, but "screen" will
be used to describe it in this document because "window" is used to mean
something else), and always has that entire screen as its suggested screen
location.  It should not be moved, resized, etc. by the program using
libuncursed (although it can be resized by the user of the program, via
resizing the terminal that uncursed is rendering onto).

`int COLORS`, `int COLOR_PAIRS` are integers that should be treated as
read-only `uncursed_color` values.  They specify the maximum number of colors
in existence, and maximum number of color pairs (palette entries) that can be
used at any one time.  In curses, these would have been dynamically decided
depending on the terminal; at present, libuncursed always gives these the
values 16 and 32767 respectively, although it is probably best to read their
values rather than hardcode the numbers in case they ever change.

`int TABSIZE`, defaulting to 8, is used to work out how far the cursor should
advance if a program attempts to output a tab character.  (It advances to the
next column that's a multiple of `TABSIZE`.)  This can be assigned to freely
by the application that uses uncursed.

`int LINES`, `int COLS` give the screen dimensions.  (You could also access
these via querying the size of `stdscr`, but the existence of these variables
is traditional and makes for shorter code.)  These will be written to by
libuncursed upon initialization and if the terminal is resized; programs that
use libuncursed should only read from them, not assign to them.

Constants
---------

These might be declared as macros, or as const global variables; don't rely on
a particular implementation.

`ERR`, `OK` are the standard return values from libuncursed functions that
return integers (unless they have some more meaningful value to return), and
represent failure and success respectively.  (There's also `KEY_CODE_YES`
which is used only by `get_wch`, because apparently ncursesw's developers
couldn't resist the concept of three-valued booleans.)

`CCHARW_MAX` gives the maximum number of characters (the base character plus
any combining characters) that an be packed into a `cchar_t`.  Note that in
practice, terminals tend to be quite limited in their support for combining
characters (even more so than in their support for Unicode in general); it's
reasonable to use combining characters if you have no other option, but in
general, it's going to be more portable if you just assume that this is 1.
Terminals each have their own effective `CCHARW_MAX` limits, which might be
lower or higher than libuncursed's; if you try to draw more combining
charaters onto a part of the screen than the terminal can handle, expect
rendering issues.

The 8 colors used by libuncursed are `COLOR_BLACK`, `COLOR_RED`,
`COLOR_GREEN`, `COLOR_YELLOW`, `COLOR_BLUE`, `COLOR_MAGENTA`, `COLOR_CYAN`,
`COLOR_WHITE`.  Terminals supported by libuncursed typically support 16
colors, but vary in the way that this is specified; some will use 8 colors,
and then a brighter set of 8 colors to render bold (making 16 in total), and
some just have 16 colors and make bold have no effect on color.  libuncursed
will transparently translate between the two methods according to what the
terminal wants; thus, if you want a bright color, you can either use the
`A_BOLD` attribute, or add 8 to a color code, or both, with all three methods
having the same visual effect.  (Going above 8 for a *background* color is not
recommended; libuncursed will attempt to tell the terminal to produce a bright
background color, but some terminals will react to this in some entirely
unwanted way, such as via producing blinking text, and there is no way that
libuncursed can detect this.)

There are likewise constants for the various text attributes.  The constants
supported by libuncursed are `A_NORMAL`, `A_BOLD`, and (on many but not all
terminals) `A_UNDERLINE`.  It also recognises some other names for
compatibility with ncurses: `A_INVIS` and `A_REVERSE` are rendered via setting
the foreground to the background, and swapping the foreground and background,
respectively (and thus do not do anything that could not be accomplished
simply using color), `A_STANDOUT` is a combination of various attributes that
increase visibility (and should be avoided in favour of something more
specific), and `A_BLINK`, `A_DIM`, `A_PROTECT`, `A_ALTCHARSET` are recognised
but do not do anything at all.

There are also `A_CHARTEXT` (a mask that can be bitwise-ANDed to a `chtype` to
get a `char`, or its complement bitwise-ANDed to a `chtype` to get an
`attr_t`); `COLOR_PAIR(x)`, a macro which constructs an `attr_t` from a color
pair number; and `PAIR_NUMBER(x)`, a macro that does the opposite, extracting
a color pair number from an `attr_t`.

A large number of constants represent special characters by name (line-drawing
characters and the like).  There are `WACS_*` constants of type `cchar_t *`;
and for constants in the first column, `ACS_*` constants of type `char` that
approximate the `WACS_*` constants as closely as possible with codepage 437
characters (for backwards compatibility with curses programs, or if you really
strongly don't care about Unicode support):

    ACS_BLOCK    WACS_BLOCK     ▮
    ACS_BOARD    WACS_BOARD     ▒
    ACS_BULLET   WACS_BULLET    ·
    ACS_CKBOARD  WACS_CKBOARD   ▒
    ACS_DARROW   WACS_DARROW    ↓
    ACS_DEGREE   WACS_DEGREE    °
    ACS_DIAMOND  WACS_DIAMOND   ◆
    ACS_GEQUAL   WACS_GEQUAL    ≥
    ACS_LANTERN  WACS_LANTERN   ☃
    ACS_LARROW   WACS_LARROW    ←
    ACS_LEQUAL   WACS_LEQUAL    ≤
    ACS_NEQUAL   WACS_NEQUAL    ≠
    ACS_PI       WACS_PI        π
    ACS_PLMINUS  WACS_PLMINUS   ±
    ACS_RARROW   WACS_RARROW    →
    ACS_S1       WACS_S1        ⎺
    ACS_S3       WACS_S3        ⎻
    ACS_S5       WACS_S5        ⎼
    ACS_S7       WACS_S7        ⎽
    ACS_STERLING WACS_STERLING  £
    ACS_UARROW   WACS_UARROW    ↑

    ACS_BTEE     WACS_BTEE      ┴    WACS_T_BTEE      ┻    WACS_D_BTEE      ╩
    ACS_HLINE    WACS_HLINE     ─    WACS_T_HLINE     ━    WACS_D_HLINE     ═
    ACS_LLCORNER WACS_LLCORNER  └    WACS_T_LLCORNER  ┗    WACS_D_LLCORNER  ╚
    ACS_LRCORNER WACS_LRCORNER  ┘    WACS_T_LRCORNER  ┛    WACS_D_LRCORNER  ╝
    ACS_LTEE     WACS_LTEE      ┤    WACS_T_LTEE      ┫    WACS_D_LTEE      ╣
    ACS_PLUS     WACS_PLUS      ┼    WACS_T_PLUS      ╋    WACS_D_PLUS      ╬
    ACS_RTEE     WACS_RTEE      ├    WACS_T_RTEE      ┣    WACS_D_RTEE      ╠
    ACS_TTEE     WACS_TTEE      ┬    WACS_T_TTEE      ┳    WACS_D_TTEE      ╦
    ACS_ULCORNER WACS_ULCORNER  ┌    WACS_T_ULCORNER  ┏    WACS_D_ULCORNER  ╔
    ACS_URCORNER WACS_URCORNER  ┐    WACS_T_URCORNER  ┓    WACS_D_URCORNER  ╗
    ACS_VLINE    WACS_VLINE     │    WACS_T_VLINE     ┃    WACS_D_VLINE     ║

Key codes
---------

The other major group of constants is key codes.  curses implementations are
historically limited to a relatively small number of recognised keypresses,
meaning that frequently if an unusual combination of keys were pressed, your
application would not be able to respond to it at all.  uncursed uses a much
larger number of keypresses than in curses; `KEY_MAX`, the highest possible
keypress code, is a much larger number than before, and thus should probably
not be used if you're trying to do something like enumerate all possible
keypresses or something like that.

Another deviation from curses is that key codes can potentially be produced
even if there is no constant for them.  This most frequently happens if the
user presses a key on their keyboard that sends a code that is correctly
formatted for the key-sending protocol in use (i.e. uncursed can parse it),
but it doesn't correspond to any key that uncursed is aware of.  In such
cases, the key will be given an otherwise unallocated code based on what
uncursed parsed from the representation of the key that it was sent; `keyname`
will return a representation of a C expression that would produce this code,
and `friendly_keyname` will return a string along the lines of `Unknown101`
that is (as far as possible) the same for the same keypress, and different for
different keypresses.  (Thus, although uncursed does not know what the key is,
and thus your application doesn't either, your application will nonetheless be
able to bind commands to the key via treating the key number as an opaque and
meaningless identifier.)

One other thing to mention is an unfortunate historical accident.  The DEC
VT100, on which the protocol used to communicate with modern terminals was
based, had a numeric keypad with four general-use keys above it (PF1, PF2,
PF3, PF4); it also had a row of function keys (F1, F2, etc.), but the first
few of these were not available to applications.  Thus, when more modern
keyboard layouts were introduced, such as the IBM PC layout used by the vast
majority of modern systems, terminal emulator designers had to make a choice
as to how to map their function keys so that they worked with the existing
programs that were designed to work with a VT100.  There were two major
designs: some terminal emulators mapped F1-F4 on the PC keyboard (which aren't
usable on a DEC) to PF1-PF4 in the protocol; and others associated the key
codes with physical locations on the keyboard, thus PF1-PF4 would be produced
via NumLock, numpad plus, numpad times, numpad divide.  Therefore, if
libuncursed sees a code of "PF1" arriving from the terminal, it can't know
whether the user actually pressed F1, or NumLock.  (Or is using a terminal
that actually has a physical PF1 key.)  In such a situation, it will return
`KEY_PF1` to the application, and leave the application to figure out what to
do with it.

There are two sets of key codes that can be generated: keys that represent
printable characters, and keys that represent other things.  When using the
Unicode function to obtain a key, `get_wch`, the two sets of key codes are
separate; `get_wch` will return `OK` and store a (`wchar_t`) Unicode codepoint
in its `wint_t *` argument if a Unicode character is entered by the user
(either as a single key, or using some sort of input method such as dead keys
or a compose key); and it will return `KEY_CODE_YES` and store a key code (an
`int`) in its `wint_t *` argument if the user pressed a key that does not
correspond to a Unicode character.  None of the codes from 0 to 255 are used
for key codes; thus, the backwards compatibility non-Unicode API, `getch`,
will return either a single character or a key code, with the value
determining which.  Most functions that deal with a `char` interpret it as a
character encoded in code page 437; however, the overwhelmingly most common
way to produce a code from 128 up in a non-Unicode terminal is to use the Meta
key (or sometimes the Alt key), which literally means "add 128 to the
character code", and thus a code of, say, 193 should typically be interpreted
as a capital 'A' (code 65) with 128 added to it, and rendered by applications
as `Meta-A`.  (It could also mean `Á`, Unicode and Latin-1 codepoint 193, but
this is less likely; perhaps the most reasonable behaviour for an application
is to use the Unicode/Latin-1 behaviour if they were expecting text to be
input, and to treat it as a Meta-modified character otherwise.)  uncursed will
attempt to, as far as possible, distinguish between Meta and Alt, to help
avoid this problem (Alt-A will, if possible, send the key code `KEY_ALT |
'A'`, rather than the printable character code 193), but on some terminals it
is impossible to distinguish.

One other point to note is the keys sent by the numeric keypad.  uncursed will
attempt to distinguish between numeric keypad keys and main keyboard keys, via
using codes like `KEY_A3` for the key on the numeric keypad that sends PgUp or
9 depending on the NumLock setting.  (On most terminals, it is not possible to
simultaneously determine the NumLock state, and also distinguish between main
keyboard and numeric keypad 9; distinguishing which 9 was pressed is more
useful to most roguelikes, which would want to use the numeric keypad 9 for
movement but not the main keyboard 9.)  There are commonly used terminals
(such as `gnome-terminal`) for which it is impossible to distinguish; thus,
the key in question would be sent as the printable `'9'` if NumLock were on,
or as the key code `KEY_PPAGE` if NumLock were off.

There are also `KEY_ALT`, `KEY_SHIFT`, `KEY_CTRL`, that can be bitwise-ORed
into any nonprintable key code.  For printable keys, Shift and Ctrl tend to
produce other printable keys; e.g. there is a code point in Unicode for
Ctrl-A, and Shift-A is just a capital 'A' (with the A key on the keyboard
normally generating a lowercase 'a').  There are no Unicode codepoints for
Alt-combinations; thus, Alt-A has its own nonprintable key code defined,
`KEY_ALT | 'A'`. (Unicode codepoints cannot normally be bitwise-ORed with
modifiers like that; however, there is a `KEY_ALT |` code defined for all
printable ASCII characters, as an exception to this rule.)

All `KEY_` macros are reserved by uncursed for future expansion.  Here are the
currently defined key codes (for nonprintable keys):

### Main keyboard keys

    KEY_HOME       Home                 KEY_END        End
    KEY_IC         Insert               KEY_DC         Delete
    KEY_PPAGE      PgUp                 KEY_NPAGE      PgDn
    KEY_UP         Up arrow             KEY_DOWN       Down arrow
    KEY_RIGHT      Right arrow          KEY_LEFT       Left arrow
    KEY_BREAK      Pause/Break          KEY_BTAB       Backtab (Shift+Tab)
    KEY_BACKSPACE  Backspace            KEY_ESCAPE     Escape
    KEY_PRINT      Print Screen

There are also `KEY_F1` to `KEY_F20` for function keys; some terminal
emulators can simulate function keys beyond F12 using key combinations (even
though keyboards rarely have more than 12 function keys nowadays), and
uncursed will report these keys literally if it receives them.  Bear in mind
that F1 to F4 might be sent as `KEY_PF1` to `KEY_PF4` instead.  `KEY_PRINT` is
also a special case, because terminal emulators typically refuse to send it;
it may be available via interface plugins other than `tty`, however.

The other common main keyboard keys send Unicode codepoints; Tab is in unicode
at code point 9, and Return is at code point 13 (carriage return), or
occasionally at code point 10 (newline), although libuncursed will attempt to
tell the terminal to send code point 13.  There's apparently a long standing
flamewar over which code Backspace should send; it generally has its own code
nowadays (which is why `KEY_BACKSPACE` exists), but some terminals will send
it as code point 8 (making it indistinguishable from control-H).  Escape also
has a Unicode code point, at 27, but given that this code point is often used
as part of a longer key code, libuncursed has to go to quite some trouble to
disambiguate, and will send `KEY_ESCAPE` for a code 27 that appears to have
been sent by the Escape key specifically.

### Numeric keypad keys

    KEY_A1  7/Home    KEY_A2  8/Up      KEY_A3  9/PgUp
    KEY_B1  4/Left    KEY_B2  5         KEY_B3  6/Right
    KEY_C1  1/End     KEY_C2  2/Down    KEY_C3  3/PgDn
    KEY_D1  0/Ins                       KEY_D3  ./Del
    
    KEY_ENTER      Enter
    KEY_NUMPLUS    +
    KEY_NUMMINUS   -
    KEY_NUMTIMES   *
    KEY_NUMDIVIDE  /

Remember that NumLock, divide, times, minus might be sent as `KEY_PF1` to
`KEY_PF4`.  (In fact, terminals that send a code for NumLock at all tend to
send it as `KEY_PF1`; thus, there is no separate `KEY_NUMLOCK`.)  Some
terminals send the same codes for numeric keypad keys as main keyboard keys;
as such, we recommend that applications treat, say, `KEY_A1` and `KEY_HOME`
identically by default (thus allowing users with these terminals to be able to
use the main keyboard and numeric keypad numbers differently simply via
turning off NumLock).

### Other key codes

If uncursed receives a key that it does not recognise, it will synthesize a
key code for it; it will have no corresponding macro, but can be treated like
an opaque identifier by the application (and will produce sensible output in
response to `keyname` and `friendly_keyname`).  There are also some key codes
that do not correspond to keys on the keyboard, but to other things that might
happen in response to a request for a key, and thus have to be returned by
`get_wch` and `getch`:

    KEY_UNHOVER  The mouse is no longer hovering over a defined hover region
    KEY_SIGNAL   A message from a signal handler or another thread
    KEY_OTHERFD  Input is available on a socket or pipe
    KEY_RESIZE   The terminal was resized
    KEY_INVALID  The terminal sent a malformed key code
    KEY_HANGUP   The terminal no longer exists

`KEY_MAX` is the highest possible key code that will be generated by
libuncursed (and, as a result of key modifier combinations and a large number
of codes reserved for unknown keys, is quite large).  You can use key codes
strictly higher than `KEY_MAX` for your own uses (such as mouse callbacks).

`KEY_SIGNAL` and `KEY_OTHERFD` are only ever sent by your own application.
Their purpose is to allow a program to accept input from multiple sources at
once; you can call `get_wch` or similar in your main loop, and it will return
`KEY_OTHERFD` if the data arrived on a file descriptor it was told to monitor,
or `KEY_SIGNAL` if you called `uncursed_signal_getch` (most likely via use of
a signal handler or a different thread).  See the documentation for
`uncursed_signal_getch` and `uncursed_watch_fd` for more information.

Note that `KEY_RESIZE` requires special handling by the application, and
behaves differently from its behaviour in ncurses.  In ncurses, when the
terminal is resized, all windows will be moved and/or resized to fit, which
normally completely garbles the screen.  In uncursed, windows don't
necessarily have to fit on the screen (they merely have a *suggested* screen
location), but if a window protrudes beyond the screen, it cannot be drawn to
the screen until it is moved or resized to fit.  An application should thus
react to `KEY_RESIZE` via moving, resizing and/or recreating windows itself,
in order to adapt to the new screen layout, before it tries to do any more
drawing.

`KEY_HANGUP` also needs special handling.  After this is returned, all
attempts to request input will return `KEY_HANGUP`, and no drawing commands
will be meaningful, meaning that the application can no longer do user
interaction.  This code is often sent during system shutdown or in other
situations where an application needs to shut down quickly, and there may be
an operating-system-imposed time limit on how much time the application will
be allowed to run, and thus the application should save or produce autosave
data (if relevant), and exit as quickly as possible.

Functions
---------

Unless otherwise stated, all functions returning `int` return `OK` on success
and `ERR` on error.

### Initialization

    void initialize_uncursed(int *p_argc, char **argv)

One of the very first things an uncursed-using program should do is to
initialize uncursed using the `initialize_uncursed` function.  This does not
do anything visible (like `initscr` would do); rather, it parses command-line
arguments and loads the appropriate libraries that will later be needed.  If
something goes wrong, such as a library not being available, it will call
`exit()` (which is a good reason to call `initialize_uncursed` right at the
start of the program).  The arguments are a pointer to your program's `argc`,
and your program's `argv`; `argc` and `argv` will be modified to remove any
arguments parsed by uncursed itself.

The requirement to call this function is the largest reason that uncursed is
not a drop-in replacement for ncurses (although the two are very similar).

    void uncursed_set_title(char *title)

Plugins that create a window need to know what title to give it, at window
creation time.  The default is "Uncursed"; if you want to set your own title,
call this routine before calling `initscr`.

    WINDOW *initscr(void)

This function tells libuncursed to take control of the terminal, and should be
called before any other uncursed functions apart from `initialize_uncursed`,
`uncursed_set_title`, and `isendwin` (variables like `stdscr` and `LINES` are
not meaningful before running this function, although for compatibility with
some other curses implementations `stdscr` will be `NULL` before `initscr` is
called; and most of the rendering and input functions will cause segfaults or
otherwise malfunction).  Under most error conditions, it will exit the program
(and if uncursed has not been initialized, it will *crash* the program with an
abort signal, because this is a fault of the program itself rather than
something like a failure to allocate memory).  Otherwise, it will return
`stdscr` (allowing you to avoid the use of global variables, if you wish).

After calling `initscr`, uncursed may gain control of stdin, stdout and
stderr; your application should not try to use these standard file handles
without first calling `endwin`, or it is likely to produce garbled output
and/or receive garbled input.

`initscr` should only be called once per program; if you try to call it a
second time, it will do nothing and return `NULL`. To tell libuncursed to take
control of the terminal a second time, after it has relinquished it, use
`refresh`.

When using interface plugins that create their own windows, rather than using
an existing terminal, the call to `initscr` is what creates the window.

    int endwin(void)

This function tells libuncursed to relinquish control of the terminal.  It
should be called just before your program exits in order to restore the
terminal's settings to normal, and can also be called at other times if your
program needs to write to stdout or stderr for some reason.  If uncursed is
using an interface plugin that creates its own window, rather than using an
existing terminal, this will have no visible effect on the screen (although
stdout/stderr will be reconnected to wherever they were connected before
uncursed was run, most likely the terminal from which the uncursed-using
program was run).  If uncursed is using an existing terminal, this will
attempt to restore as much of its state as possible to the state when
`initscr` was called.

To undo the effects of `endwin`, use `refresh`.  After `endwin` is called,
`refresh` should be the next uncursed function to be called (not counting
calls to `isendwin`, and unless the program just exits); anything else is
undefined behaviour, and may garble the terminal or segfault.

    uncursed_bool isendwin(void)

Returns a nonzero value if `endwin` has been called more recently than
`refresh`, or if `initscr` has not been called yet, or a zero value otherwise.
This function is an exception to the general rule that uncursed functions can
only be called if uncursed has control of the terminal (because its purpose 
is to determine whether uncursed has control of the terminal).

    void set_faketerm_font_file(char *filename)

Some uncursed interface plugins create their own window for rendering, perhaps
because no acceptable terminal is available on the system.  In such cases,
there is a chance that there might not be an appropriate font available
either, and thus such plugins *require* the use of this function (which should
be called after `initscr` but before the first `refresh`) in order to specify
which font to use.  A font file should be a PNG image that contains the 256
characters of code page 437, 16 to a row and 16 rows high, in codepoint
order.  (All characters should be the same size, meaning that the dimensions
of the image will both be divisble by 16.)  The image should use a transparent
background, and draw the characters themselves in white.

This function will be ignored by interface plugins that do not create a fake
terminal; and if something goes wrong reading the file, it will print
explanatory messages to the terminal from which the program was loaded, if any
(because it cannot print to its own window), and continue to run but with no
characters visible.

### Input

    int [w]get_wch([WINDOW* win], wint_t *codepoint_or_keycode)
    int [w]getch([WINDOW* win])
    int timeout_get_wch(int milliseconds, wint_t *codepoint_or_keycode)

Performs input; normally this will return a codepoint or a key code, although
it can occasionally return other values like `KEY_RESIZE` or `KEY_HANGUP`.
The Unicode version, `get_wch`, returns the input value in the `wint_t`
pointed to by its last argument, and returns `OK` if it input a Unicode
codepoint (in which case the `wint_t` value is a `wchar_t`, `KEY_CODE_YES` if
it input a key that does not have a Unicode codepoint (in which case the
`wint_t` value is an `int` that holds a key code value), or `ERR` if there was
no input (typically because the window in question has a timeout set and there
was no input within the given timeout).  `getch` will return a key code value,
an ASCII or meta-ASCII character, or ERR (these ranges do not overlap); it
returns CANCEL CHARACTER (0x94) if the input was a Unicode character that does
not fit into the Latin-1 range.

Should these functions return `KEY_RESIZE` (because the user resized the
terminal), `stdscr` will be resized to match the new terminal size.  Thus, a
program must be very careful about calling drawing functions after a
`KEY_RESIZE`, until it has moved all its windows back within the bounds of the
screen.

The window argument is a legacy from ncurses.  For `getch` (but for ncursesw
compatibility, not `get_wch`), `stdscr` or the window specified will be
`refresh`ed before performing any input; additionally, the length of time to
wait before giving up and returning `ERR` is a property of the specified
window.  If you prefer your input to be separate from your output, you can use
the uncursed-specific function `timeout_get_wch`, which specifies the timeout
to use explicitly rather than considering it to be a property of a window; use
a value of -1 to wait indefinitely for a key, and otherwise a timeout in
milliseconds.  It is otherwise the same as `get_wch`.

    int [w]timeout([WINDOW *win], int milliseconds)

`getch` and `get_wch` use the timeout of a window in order to know how long to
block for before returning `ERR`. curses has a great range of functions that
can be used to set this, but the preferred method for `uncursed` is the
`timeout` function, which sets the timeout for a window; -1 means an infinite
timeout (blocking until `getch` has something to return), 0 means to return
immediately if no keypresses are already waiting, and positive values set a
timeout in milliseconds.  (Other functions from curses that set the timeout,
and are implemented by uncursed for backwards compatibility but should not be
used in new programs: `halfdelay` which measures in tenths of a second;
`nodelay` that can only set zero or infinite timeouts; and `nocbreak` which
has a lot of effects under curses, such as turning on line-at-a-time input,
but whose only effect under uncursed is to set `stdscr`'s timeout to
infinite.)

    int unget_wch(wchar_t codepoint)
    int ungetch(int codepoint)

These functions (Unicode and ASCII respectively) cause the next input request
to return the given codepoint (along the same lines of `ungetc` for `getc`).
They return `ERR` if there is already a character waiting to be pushed back,
`OK` on success.

    wchar_t *wunctrl(wchar_t codepoint)
    char *unctrl(char codepoint)
    char *key_name(wint_t keycode)
    char *keyname(int keycode_or_codepoint)
    char *friendly_keyname(int keycode_or_codepoint)

These functions each return strings (which do not have `const` for backwards
compatibility, but which should be considered read-only), that describe the
name of a character.  (The return value might be a literal string; it might
also be a pointer to a static buffer, so it should not be assumed to be valid
after the next call to one of these funtions.) `unctrl`, `wunctrl` will give a
printable name for ASCII and Unicode codepoints respectively (which is
typically just the codepoint itself, but which might be, say, `^A` or `M-e`
for control- or meta-modified keys); `key_name` gives a name (of the form
`KEY_LEFT`, i.e. it descrites a key code constant) for a key code.  `keyname`
will operate either on (ASCII) codepoints, or on (ASCII or Unicode) key names
(i.e., it can handle the values that `getch` outputs); and `friendly_keyname`
is similar, but gives a more human-readable name (like `Left` or `PrtSc`).

### Input from sources other than the terminal

uncursed does not input from sources other than its terminal, but it has
functions to simplify its use by programs that take input from multiple
sources, such as network sockets or signal handlers.  (Trying to get a curses
program to read from either the terminal or a network socket is very complex;
the simplest method I know of involves multithreading, which is not very
simple.)

    void uncursed_signal_getch(void)

Causes the current or next call to `get_wch`, `getch`, or `timeout_get_wch` to
return as if `KEY_SIGNAL` were pressed.  Unlike every other uncursed function
(which must be called on the same thread as `initialize_uncursed` and
`initscr`), this function can safely be called from a signal handler, or a
thread other than the thread that the other uncursed functions are running on
(and in fact, this is the only way to call it during a call to `get_wch`).  If
this function is called multiple times between calls to `get_wch`, its effects
will be queued, causing the same number of calls to `get_wch` to return
`KEY_SIGNAL` (although those calls will not necessarily be consecutive,
because they can be overriden by other unusual return values, such as
`KEY_RESIZE` and `KEY_HANGUP`).  There may be OS-dependent limits on how many
signals can be queued up (likely in the low hundreds); when the limit is
reached, the function may block, or else signals may be lost.

The intended use for this is to call `get_wch` in the main loop of your
program, while a signal handler or separate thread waits in the background.
When an event arrives that your main loop would need to know about, the signal
handler or separate thread pushes information about it into an appropriately
synchronized queue, then calls `uncursed_signal_getch`; the main loop will
then read the `KEY_SIGNAL` and pop the information it needs from the queue.

    void uncursed_watch_fd(int fd)
    void uncursed_unwatch_fd(int fd)

`uncursed_watch_fd` causes future calls to `get_wch`, `getch`, or
`timeout_get_wch` to return `KEY_OTHERFD` when input is available on the given
file descriptor; `uncursed_unwatch_fd` undoes this effect.  What constitutes a
"file descriptor" depends on your operating system; on Windows, the only
supported file descriptors are network sockets, and on POSIXy systems like
Linux, Mac OS X, and BSD, descriptors for open pipes/fifos are also supported,
as well as files representing character devices.  "Input is available" means
that it is possible to read from the given file descriptor without
blocking.  (Note that a file at EOF can usually be read without blocking; the
read is not particularly interesting, but it doesn't block.)

When using libuncursed on Windows, watching a network socket will, as a side
effect, cause it to go into nonblocking mode: this is a deficiency of the
Windows API.  (It may be simplest to manually set the socket to nonblocking
mode on every operating system, to avoid having to deal with this special
case.  Note, however, that it is irrelevant whether the socket is blocking or
not if you only read from it upon receiving a `KEY_OTHERFD` response, because
it will already have been established that the reads cannot block.)

The `KEY_OTHERFD` return does not specify *which* file descriptor can be read
from without blocking, so if you are watching more than one file descriptor,
you will need to determine which one is readable via use of nonblocking reads,
or perhaps the POSIX API function `select()`.

The given file descriptor, `fd`, may be limited in range, due to the system
calls used to monitor a descriptor for input.  (For instance, it may need to
be less than `FD_SETSIZE` on a POSIXy system.)  It must also be in a state in
which it would reasonably be possible to accept input from it (i.e. open and
connected, for a network socket), and should not be closed until after it is
unwatched.

The intended use for this is to be able to receive data from either the user
or from a network connection; you call `uncursed_watch_fd` when you open the
network connection, then read from it whenever the `get_wch` in your main loop
returns `KEY_OTHERFD`.

### Drawing windows to the screen

One of the major purposes of uncursed is to draw on the screen.  The way in
which this is accomplished is to first draw on windows (which happens in
memory and produces no output on the screen), and then to copy the windows to
the screen, which is the only point at which the screen actually updates.

    int [w]refresh([WINDOW *win])

This function copies the contents of the specified window (or `stdscr`) to the
screen, at its suggested screen location; and moves the screen's cursor to the
same location as the window's cursor.  (These routines must not be called,
directly or indirectly, for a window that does not fit on the screen; doing so
will likely corrupt memory or cause a segfault.  This is a good reason to move
windows so that they fit on the screen immediately after receiving a
`KEY_RESIZE`.)  As usual, they return `ERR` on error, `OK` on success.

`refresh` additionally can be used to undo the effects of `endwin`, once again
giving uncursed control of the terminal.

    int redrawwin(WINDOW *win)
    int wredrawln(WINDOW *win, int first, int count)

These functions cause uncursed to forget any assumptions that it might have
about the contents of the screen behind the suggested screen location of the
specified window (or the suggested screen location of a particular range of
lines of that window). This will cause the area behind the window to be
repainted from scratch at the next refresh.

    int wnoutrefresh(WINDOW *win)
    int doupdate(void)

When several windows are to be copied to the screen at once, it is possible to
gain some extra performance via performing the updates simultaneously.
`wnoutrefresh` is identical to `wrefresh`, except that the update to the sreen
is postponed until the next call to a function that would update the screen,
or to `doupdate`, at which point all the updates happen at once.

### Attributes and attributed characters

Each character on the screen has some number of codepoints (typically 1, but
potentially more in the case of combining characters), and also some
attributes (color, boldness, etc.).  Some of the APIs in uncursed use these
attributed characters direcly (`cchar_t` for the Unicode APIs, `chtype` for
the curses compatibility APIs); others treat the character (`wchar_t` or
`char`) separately from the attribute (`attr_t`).

Attributes such as bold simply have their own constants (such as `A_BOLD`),
listed earlier.  For colors, a palette is used; there are a number of
customizable "color pairs", each of which has a background and a foreground
color, and the color pairs can be converted from `uncursed_color` pair numbers
to `attr_t` attributes using the `COLOR_PAIR` macro.  Changing the definition
of a color pair will (at the next screen refresh) recolor characters that use
that pair.

    int init_pair(uncursed_color pairnum,
                  uncursed_color fgcolor, uncursed_color bgcolor)
    int pair_content(uncursed_color pairnum,
                     uncursed_color *fgcolor, uncursed_color *bgcolor)

`init_pair` sets the color that the color pair `pairnum` represents, to the
given foreground color `fgcolor` and background color `bgcolor`.
`pair_content` retrieves the current definition of a color pair `pairnum`,
storing its foreground and background color in `*fgcolor` and `*bgcolor`.

There are also some curses backwards compatibility functions, `start_color`,
`init_color`, `has_colors`, `can_change_color`, `color_content`; none of these
do anything useful in uncursed, simply returning a reasonable return value.

    int getcchar(const cchar_t *cchar, wchar_t *characters,
                 attr_t *attr, short *pairnum, void *unused)
    int setcchar(cchar_t *cchar, const wchar_t *characters,
                 sttr_t attr, short pairnum, void *unused)

These are the accessor and mutator functions for `cchar_t` structures. `cchar`
is the `cchar_t` to access or mutate; `characters` is a null-terminated string
of `wchar_t`s (the first character is a regular character that moves the
cursor when printed, and all the other characters must be combining
characters); `attr` is the attributes of the character; `pairnum` is the color
pair number (`short` not `uncursed_number` for ncursesw compatibility); and
`unused` is ignored (as it is in every curses implementation I know of; XSI
Curses added it to allow for future expansion, and it has not yet been given
any meaning).  Normally, `getcchar` will place the fields of the `cchar_t`
into the corresponding arguments, and `setcchar` will place the arguments into
the fields of the `cchar_t`, but there are two exceptions.  First, if
`getcchar` is given a null pointer for `characters`, it will return the length
of the string it would unpack there (rather than the usual `OK`/`ERR`), and
perform no other action; second, any color pair in `attr` will be ignored by
`setcchar`, in favour of the color pair number in `pairnum`.  (`getcchar` will
include the color pair portion of `cchar` in both `pairum` and `attr`.)

    int [w]attrset([WINDOW *win], attr_t attr);
    int [w]attron([WINDOW *win], attr_t attr);
    int [w]attroff([WINDOW *win], attr_t attr);
    int [w]color_set([WINDOW *win], uncursed_color pairnum);
    int [w]attr_get([WINDOW *win], attr_t *attr, uncursed_color *pairnum,
                     void *unused);

When drawing to a window, the attribute used for newly drawn characters is
based on the character's attributes and the window's default attributes
(boolean attributes are simply bitwise-ORed together; it is not recommended to
set a nonzero color pair number on both the character and the window).
`attrset`/`wattrset` will set the default attributes for `stdscr` or for
another window to the given value; `attron` and `attroff` include or exclude
the given attributes from the window's set of default attributes, leaving any
default attributes that weren't mentioned the same.  None of those functions
should be used to set a color pair attribute (it will frequently work in
uncursed, but not if you try to set two color pairs simultaneously for the
same window, nor in other curses implementations); use `color_set` to set a
window's default color pair. `attr_get` allows you to query a window's current
default attributes.

For curses compatibility, there are also functions `attr_set`, `attr_on`,
`attr_off` that take an extra unused `void *` argument and are otherwise
identical to the versions without the underscores; and functions `standout`,
`standend` that are equivalent to `attron(A_STANDOUT)` and `attrset(A_NORMAL)`
respectively.

### Printing characters, attributes and strings

    int [w]move([WINDOW *win], int y, int x)

Most output functions with no `mv` prefix draw at the window's current cursor
location (actually, they draw at the window's current cursor location even
with the prefix, but the prefix sets the cursor location before doing anything
else, and thus the previous location is irrelevant).  This function sets that
location.  (The top-left corner of the window is (0,0).)

    int [mv][w]add_wch([WINDOW *win], [int y, int x], const cchar_t *ch)
    int [mv][w]addch([WINDOW *win], [int y, int x], chtype ch)
    
    int [mv][w]add_wch[n]str([WINDOW *win], [int y, int x],
                             const cchar_t *str, [int length])
    int [mv][w]addw[n]str([WINDOW *win], [int y, int x],
                          const wchar_t *str, [int length])
    int [mv][w]addch[n]str([WINDOW *win], [int y, int x],
                          const chtype *str, [int length])
    int [mv][w]add[n]str([WINDOW *win], [int y, int x],
                         const char *str, [int length])

`add_wch` and `addch` output a single attributed character at the cursor
location; the other four families functions output a string of attributed or
non-attributed characters at the cursor location.  (All these functions can
use a `mv` prefix to move the cursor first, and/or a `w` prefix to output to a
window other than `stdscr`; the string functions use an `n` infix to bound the
length of the string, much like `strnlen` does.)

In general, the functions that use attributed characters will ignore the
current default attribute of the window, whereas the functions that just use
plain `wchar_t` or `char` wil respect it.  The `addch` family is an exception,
for curses backwards compatibility; it will combine the window's default
attributes with the attributes given (which might lead to bizarre results if
either has a nonzero color pair).  Sticking to the Unicode functions, as
recommended, will allow you to avoid this special case.

Outputting characters normally moves the cursor; backspace (codepoint 8) moves
it backwards, tab (codepoint 9) moves it to the next multiple of `TAB_STOP`,
newline (codepoint 10) moves it to the start of the next line, and other
characters move it one space to the right (wrapping onto the next line when
the edge of the window is reached). The `addchstr` family does not move the
cursor, however, nor does it interpret cursor movement commands. The other
string-related families allow a length of -1 to stop at the edge of the
window, rather than wrapping onto the following line.

There are also `echo_wchar`, `echochar` variants of `add_wch`, `addch`, that
call `refresh` (or `wrefresh` for `wecho_wchar` or `wechochar`) after
outputing the character, and otherwise work identically.

    int [mv][w]printw([WINDOW *win], [int y, int x], const char *format, ...)
    int vw_printw(WINDOW *win, const char *format, va_list args)

These functions are the uncursed equivalents of `printf` and `vprintf`
respectively; they take a `printf`-like format string, and arguments to fill
in the format specifiers of that string (either as separate arguments for
`printw`, or as a `va_list` for `vw_printw`), and print the characters one at
a time using the window's default attribute.  Sadly, there are no Unicode
versions available at the moment.

    int [mv][w]chgat([WINDOW *win], [int y, int x], int len,
                     attr_t attr, uncursed_color pairnum, void *unused)

In addition to the functions for drawing strings of characters, there is also
a function for drawing strings of attributes; `chgat` will replace `len`
attributes from the cursor position with the given atribute `attr` and color
pair `pairnum` (with `unused`, as always, being ignored).  The cursor does not
move as a result of the attributes being drawn (although, of course, it can
move as a result of any `mv` prefix).

### More complex drawing

    int [w]border_set([WINDOW *win],
                      const cchar_t *left_side,   const cchar_t *right_side,
                      const cchar_t *top_side,    const cchar_t *bottom_side,
                      const cchar_t *top_left,    const cchar_t *top_right,
                      const cchar_t *bottom_left, const cchar_t *bottom_right)
    int box_set(WINDOW *win,
                cchar_t *vertical_sides, cchar_t *horizontal_sides)
    int [mv][w]hline_set([WINDOW *win], [int y, int x],
                         const cchar_t *rendition, int length)
    int [mv][w]vline_set([WINDOW *win], [int y, int x],
                         const cchar_t *rendition, int length)

These functions draw lines across, or boxes around, windows.  (There are also
non-Unicode versions for backwards compatibilty; these have no `_set` suffix,
and use `chtype` rather than `const cchar_t *`.)  The given renditions will be
used to draw the relevant sides of a box, or the characters that make up a
line (the window's current default attribute is ignored); for drawing lines,
the left or top end of the line starts at the current cursor position (which
does not move), and the line will be drawn to the given length, or as much as
will fit into the window.

### Clearing and copying windows

    int [w]erase([WINDOW *win])
    int [w]clrtobot([WINDOW *win])
    int [w]clrtoeol([WINDOW *win])

These funtions replace the entire window contents, the window contents from
the cursor onwards (the rest of the line, and all lines below), and the rest
of the line (respectively) with blanks (spaces with no attributes).

    int clearok(WINDOW *win)
    int [w]clear([WINDOW *win])

The `clearok` function causes the entire screen to be repainted from scratch
the next time the given window is `refresh`ed; `clear` is a combination of
`erase` and `clearok`.  These functions would typically be used if the
application's user had told the application that the screen was corrupted, or
requested a full screen redraw, using a key combination designated for the
purpose.

    int overlay(WINDOW *from, WINDOW *to)
    int overwrite(WINDOW *from, WINDOW *to)
    int copywin(WINDOW *from, WINDOW *to, int from_miny, int from_minx,
                int to_miny, int to_minx, int to_maxy, int to_maxx,
                int skip_blanks)

These functions copy the contents of one window onto another, or (as an
extension to curses) between two non-overlapping rectangles of the same
window.  `overlay` and `overwrite` each copy the entire source window onto the
destination window, at the top-left corner, except that `overlay` does not
copy any spaces (regardless of their attributes), leaving the original
contents of the window intact.  `copywin` allows an arbitrary rectangle to be
copied onto an arbitrary window of the same size (if the windows being copied
share storage, the retangles cannot overlap); `skip_blanks` controls whether
spaces are copied or not.

### Scrolling and shifting window contents

    [w]insdelln([WINDOW *win], int distance)
    [w]scrl([WINDOW *win], int distance)
    [w]deleteln([WINDOW *win])
    [w]insertln([WINDOW *win])
    scroll(WINDOW *win)

These functions all shift part of the window contents vertically. `insdelln`
will insert (with a positive argument) or delete (with a negative argument)
the given number of lines at the current cursor y position, shifting the part
of the window below the cursor downwards or upwards to accommodate the change;
`scrl` is similar, but will scroll the entire window upwards with a positive
argument, or downwards with a negative argument.  Added lines will be filled
with blanks (spaces with no attributes).

    [mv][w]delch([WINDOW *win], [int y, int x])

This function will delete the character at the current window cursor location,
scrolling the rest of the line to the left to compensate (and adding a blank
at the end of the line).

    scrollok(WINDOW *win, bool ok_to_scroll)

In addition to scrolling a window manually, windows can also scroll
automatically when text is written past their last line.  The `scrollok`
function can be used to turn this behaviour on or off.

### Window management

All these funtions that return a `WINDOW *` return `NULL` on error.

    WINDOW *newwin(int height, int width, int top, int left)
    WINDOW *subwin(WINDOW *parent, int height, int width, int top, int left)
    WINDOW *derwin(WINDOW *parent, int height, int width, int top, int left)

These funtions create new windows; `newwin` creates a window that does not
share its storage with any other window, whereas `subwin` and `derwin` create
windows that share their storage with the given window (and thus, drawing to
either window will automatically change the contents of the other).
Subwindows created via `subwin` or `derwin` must be destroyed before their
parent windows can be.  For `newwin` and `subwin`, the `top` and `left` values
are relative to the screen (with (0,0) being the top-left corner), and a value
of 0 for `height` or `width` means to make the window touch the bottom or
right edge of the sreen respectively.  For `derwin`, the `height` and `width`
must be positive, and the `top` and `left` values are measured relative to the
parent window's suggested screen location. (Remember that although the
`height` and `width` are important and determine how much storage is allocated
for the window, the `top` and `left` are only suggestions, and only relevant
upon drawing the window to the screen.)

    WINDOW *mvderwin(WINDOW *win, int top, int left)

When creating a subwindow, it is also necessary to specify how the parent's
and child's memories overlap.  The child's memory will have its top-left
corner at the given coordinates relative to the parent's memory.  For
subwindows created via `derwin`, the default location is the `top` and `left`
given when creating the window; for subwindows created via `subwin`, the
default location is the top-left corner of the parent's memory.

It is possible for a subwindow's memory to be outside the bounds of its
parent's, as a result of `mvderwin` or `wresize` calls (or `stdscr` being
resized because the user resized the terminal).  When this happens, any
attempt to write to or read from the subwindow may corrupt memory or cause a
segmentation fault; the situation should be rectified via use of `mvderwin` or
`wresize` before any drawing or updating is done on the subwindow.

    int mvwin(WINDOW *win, int top, int left)

This function changes a window's suggested screen location; it works on both
windows, and subwindows.  It will error (and not move the window) if you try
to set a suggested screen location that is not on the screen, based on the
curses specification; such locations are meaningful, but normally unintended
because such windows cannot be drawn.

    int delwin(WINDOW *win)

This function deletes a window.  (If it has any subwindows, those must be
deleted first.)

    int wresize(WINDOW *win, int height, int width)

This funtion resizes a window.  The contents of the window will stay the same
as far as possible, keeping the top-left the same; resizing a subwindow will
fill any gained space with contents from its parent, resizing a non-subwindow
will fill any gained space with blanks.

    void wcursyncup(WINDOW *win)

curses has several functions (`wsyncup`, `wsyncdown`, `syncok`) for
synchronizing windows and their subwindows; in uncursed, windows and their
subwindows are automatically synchronized, so this is an unnecessary operation
(and those functions are no-ops).  However, uncursed (as with curses) allows a
subwindow's cursor location to be in a different location from its parents'.
`wcursyncup` will recursively set the cursor location of all the parents of a
subwindow to be the same as that of the subwindow itself.

### Tiles support

uncursed supports mostly transparent support for graphical tiles interfaces,
allowing tiles to be easily added to a text-based program.  (This is intended
for use by games and especially roguelikes, which motivated the creation of
uncursed.)  All these functions (except `get_tile_dimensions`) are effectively
no-ops in a terminal-based interface; they can create, delete, etc. tiles
regions, but those regions will never be drawn to the screen.

    void set_tiles_tile_file(char *filename, int height, int width)

This function sets the image file that will be used for the graphical tiles,
and the dimensions of each tile within the image.  (Tiles support is off by
default; it is turned on or off using this function, using NULL for the
filename to turn it off.)  Changing the tileset will have an effect only on
the tileset used by future calls to `set_tiles_region`; it wil not affect
currently exiting tiles regions.

    void [w]set_tiles_region([WINDOW *win],
                 int tiles_h, int tiles_w, int tiles_t, int tiles_l,
                 int char_h, int char_w, int char_t, int char_l)
    void [w]delete_tiles_region([WINDOW *win])

Each window can have a "tiles region", that tells uncursed where to draw the
tiles.  After the window argument, the first four arguments give the height,
width, top and left of the area of the window that will be covered by the
tiles; the last four arguments give the area of the window that contains the
ASCII/Unicode representation that is used for non-tiles interfaces.  (In other
words, your application draws characters, with associated tiles, to the second
rectangle; the characters appear in the second rectangle, and the tiles are
drawn in the first.)  For most useful uses of this, the first rectangle would
entirely contain the second, and in many cases, the two will be identical.

If the tiles are larger than the characters in their ASCII/Unicode
representations (which is likely), and the tiles rectangle is not bigger than
the character rectangle by enough to compensate, then only part of the
character region can be shown in the tiles region.  The part that is shown
will depend on the window's cursor location, scrolling to attempt to keep the
part of the tiles region that corresponds to the part of the character region
under the cursor in view, while the cursor is in the character region.  (The
location of the tiles region is relevant only to determine where on the screen
the tiles go; it is ignored for all other purposes.)

`delete_tiles_region` removes the tiles region of a window.  (It is also
removed if the window is deleted, resized, or changes suggested screen
location.)

    void get_tile_dimensions(int down, int across, int *height, int *width)

There is no particular reason in libuncursed why the tiles have to be the same
size as the underlying characters (this distinguishes it from terminals such
as Ebonhack which replace characters with tiles directly).  Tiles-aware
programs may thus want to produce a tiles region that is large enough to hold
all the tiles (and thus is presumably larger than the character region).  This
function is given the dimensions of a character region in `down` and `across`
(i.e. this is the number of *tiles* you want to appear in the tiles region,
which is equal to the number of *characters* in the corresponding character
region); it will assign to `*height` and `*width` to specify the minimum size
of a tiles region that could fit the given tiles without scrolling (i.e. this
is the number of *characters* that will be invisible due to being behind the
tiles region).  The function looks at nothing other than the tile file to
determine this; it has no side effects, and in particular, there is no
requirement that any tiles regions actually exist.

If uncursed is using a non-graphical interface plugin, it will set `*height`
and `*width` to `down` and `across` respectively; this is because there is no
visible tiles region in such a case, with the character region serving the
purpose that the tiles region normally serves, and thus this gives the most
accurate possible indication of how much room the "tiles" (actually just
characters) take up on screen.

    int [mv][w]set_tiles_tile([WINDOW *win], [int y, int x], int tile)

Sets the tile that corresponds to the character under the cursor; does nothing
if the cursor is not in the window's character region.  Does not move the
cursor.  If the tiles are partially transparent, it can be meaningful to call
this multiple times in a row without moving the cursor; the first tile drawn
(which should be opaque) will be placed at the bottom, and subsequent tiles
nearer the top.

There is no way to *clear* a tile from a character, except by deleting the
tiles region altogether (although you can overwrite a tile with another tile,
if that other tile is opaque).  When a tiles region is introduced, every
character in the matching character region should be given an associated tile,
otherwise what is drawn in the corresponding part of the tiles region will be
undefined.

Windows with a tiles region may not be copied to or from using `copywin` or
related functions.

### Mouse support

uncursed supports mostly transparent support for a mouse.  (Programs should
not rely on a mouse being available.)  It understands six sorts of mouse
input:

    uncursed_mbutton_left      Left click
    uncursed_mbutton_middle    Middle click (or pushing in on the mouse wheel)
    uncursed_mbutton_right     Right click (or sometimes control-left click)
    uncursed_mbutton_wheelup   Mouse wheel moved upwards
    uncursed_mbutton_wheeldown Mouse wheel moved downwards
    uncursed_mbutton_hover     Mouse pointer moved onto the target

uncursed's API is rather different from ncurses' mouse API; instead of
returning `KEY_MOUSE` on every sort of mouse input (no matter where it
happens), you can customize what sort of return value you want from `getch`
and friends for different sort of mouse input over different text.  For
instance, you can write a string that sends `a`, or `KEY_LEFT`, or even
`KEY_MAX + 1` to the screen when clicked on.  This respects windows being
covered by other windows; if you write non-mouse-active text above
mouse-active text (say by refreshing the mouse-active window, then refreshing
an overlapping non-mouse-active window), clicking on it will have no effect.

    void uncursed_enable_mouse(int enable)

Use this function to enable (`enable == 1`) or disable (`enable == 0`) mouse
input.  (You probably want to put this under user control; while mouse input
is being used, the mouse will not be usable for any commands that a terminal
might normally use it for, such as selecting text.)

This function only controls the actual communication with the user.  You can
set text as mouse-active even with mouse input turned off; the text will be
mouse-active but that mouse-active status will not do anything.  This is
potentially useful, because the mouse-active status will remain if mouse input
is turned back on, so there's no need to redraw all your mouse-active text
when changing mouse state.

    void [w]set_mouse_event([WINDOW *win], enum uncursed_mousebutton button,
                            wint_t char_or_keycode, int which)

This function changes whether future text drawn in the given window is
mouse-active, and in which way.  You can think of mouse activity like an
attribute; "mouse-active text" is not really any different from, say, "blue
text" or "underlined text".  `button` is the mouse action you want the text to
respond to; `char_or_keycode` is a Unicode character or `KEY_*` constant; and
`which` is `OK` if `char_or_keycode` is a Unicode character, `KEY_CODE_YES` if
`char_or_keycode` is a `KEY_*` constant, or `ERR` to disable mouse activity
for the given button (in future-drawn text).  The standard use of the function
would be to call `wset_mouse_event` just before drawing text to a window, draw
the text, then do another `wset_mouse_event` call to undo the effects of the
first call.  The mouse activity that is set via `set_mouse_event` is also
applied to any regions of the window that are cleared or erased; this is
useful if, for instance, you want the entire window to respond to mouse wheel
movement, rather than individual lines of text within it.

For the most mouse actions (all actions except hovers), doing the appropriate
action while the cursor is above the mouse-active text will cause `get_wch`
and similar functions to return the specified character or keycode.  Hovers
are slightly more complex; the specified character or keycode will be sent
whenever the mouse is moved onto the hover-active text (whether or not it was
previously over hover-active text), including from character to character
within the hover-active text; and if the mouse is moved off hover-active text
onto non-hover-active text, this will cause `get_wch` to return `KEY_UNHOVER`.

You can use key codes higher than `KEY_MAX` for your own purposes; for
instance, you could do this to give every character on the screen a unique
return for clicks or hovers.  This is the only circumstance under which
`get_wch` and friends will return values above `KEY_MAX`.  (`KEY_MAX` itself
should not be used; it might potentially be a valid key).  However, you can
also use keys that already have an existing meaning, in which case the mouse
click will be indistiguishable from the keypress.  (This is useful if, say,
you have a status bar saying something like "Control-X: exit"; you could make
the entire text click-active, sending a Control-X.  It's also the easiest way
to make menus click-active.)

When using tiles combined with the mouse, a tile in a tiles region will be
mouse-active if the corresponding character in the corresponding character
region is mouse-active.  This preserves the standard API convention in which
the tiles region is entirely driven by a character region (typically behind
it).

    void uncursed_clear_mouse_regions(void)

This function effectively places a transparent, non-mouse-active layer over
the entire screen, causing anything currently on it to cease to respond to
mouse input.  Any mouse-active text in windows will remain mouse-active,
though, and cause the area to become mouse-active again when they are next
redrawn.

The intended use for this is when producing modal dialog boxes; you can clear
mouse regions just before drawing your pop-up box, causing clicks outside it
to have no effect (rather than sending keys that might potentially be
misinterpreted because the context has changed).  Once the dialog box is
closed, your program can just refresh all the other windows on the screen (as it
normally would upon closing a dialog box), causing them to become correctly
mouse-active again.

### Miscellaneous

    int curs_set(int visibility)

This function changes whether the cursor is visible.  Technically, the legal
vaues are 0 (invisible), 1 (visible), 2 (very visible); however, 1 and 2 tend
to have the same implementations in interface plugins in practice.

    int raw(void)
    int noraw(void) /* or */ int cbreak(void)

These functions control how the terminal should react to key combinations that
normally have a special meaning to it (a common combination is Control-C,
which on many operating systems will exit the currently running program).  In
curses, the difference between `raw` and `cbreak` modes is that `raw` mode
will attempt to send these codes to the application, whereas `cbreak` will
send them to the terminal; uncursed respects this distinction, treating the
modes the same way.  (Unlike curses, uncursed is always in either raw or
cbreak mode; it cannot be shifted into, say, cooked mode.)  Some terminals or
operating systems may have key combinations that override even raw mode (such
as Control-Alt-Delete on Windows which cannot be intercepted by applications,
or Alt-SysRq-R on Linux which has the specific purpose of forcibly taking
terminal applications out of raw mode).

    int beep(void)
    int flash(void)

These functions trigger an audible and visible bell, respectively.

    int delay_output(int milliseconds)

This function causes a delay of the given number of milliseconds.  (ncurses
uses pipelining for this: the delay can happen on the terminal in parallel
with your programming doing processing.  libuncursed might implement this
optimization in future, but at present, it doesn't.)

    int use_default_colors(void)
    int assume_default_colors(int foreground, int background)

These functions (which use `int` rather than `uncursed_color` for ncurses
compatibility) allow the application to change the meaning of color pair 0.
`assume_default_colors` sets the foreground and background of color pair 0 to
the given values (-1 means the default value for the terminal);
`use_default_colors` sets them both to the default value for the terminal.

Note that on many terminals, the default color is not one of the 16 colors in
the terminal palette (and it can even be something bizarre like "transparent"
or "background image"), and it may (in fact, normally does) look different as
a foreground and as a background; note also that setting a foreground color
but not a background color, or vice versa, is a recipe for having some users
unable to see your text because its color is too similar to the background.

After calling `use_default_colors`, -1 also becomes accepted as a color value
for init_pair.  (In fact, uncursed also accepts it even without calling
`use_default_colors`, but this is incompatible with ncurses and thus should be
avoided.)

### Implemented but ignored for curses compatibility

The following extra curses functions are implemented by uncursed with
appropriate signatures and return values, but do not do anything useful
(mostly because they request uncursed to do something it would do anyway, or
because their only effect is to tell curses that a specific optimization is
reasonable):

`touchwin`, `touchline`, `untouchwin`, `wtouchln`, `nonl`, `leaveok`,
`typeahead`, `notimeout`, `qiflush`, `noqiflush`, `meta`, `keypad`,
`intrflush`, `noecho`.


Writing a new plugin for uncursed
=================================

libuncursed is eventually intended to support multiple kinds of plugins, but
at the moment it only supports interface plugins (as they are known
internally, "input plugins", because they produce input to the program in
addition to handling output).

Plugins should include the `"uncursed_hooks.h"` file, create a new `struct
uncursed_hooks` structure, and send it to the main part of libuncursed via the
`uncursed_hook_list` variable.  (To accomplish this, a C++ wrapper file is
used that does the importing, because C files cannot have side effects upon
being loaded; the wrapper is both very simple and highly regimented in
structure, so it's best to simply copy and modify an existing wrapper.)

The structure contains a number of function pointers, which allow libuncursed
to call into the plugin (conventionally named with the plugin name and then
`_hook_`, as in `tty_hook_init`, although this is not required); the plugin
can call back into libuncursed via routines with names starting
`uncursed_rhook_`.

Calls from libuncursed into a plugin
------------------------------------

Each of these is a function pointer stored in a field of the plugin's
`uncursed_hooks` structure.

### Initialization, shutdown

    void (*init)(int *lines, int *cols, char *title)
    void (*exit)(void)

The `init` function implements `initscr` and the `endwin`-cancelling effect of
`refresh`, and tells the plugin to do what is necessary to start up and (if
necessary) take control of the terminal.  The `lines` and `cols` arguments
should be filled with the size of the terminal, and will be used by
libuncursed to allocate data structures, as well as being available to a user
of uncursed in the `LINES` and `COLS` variables.  `title` specifies the window
title, for ports that care.  `exit` handles any shutdown code required to
relinquish control of the terminal.  (For plugins that create fake terminals,
it should be a no-op, or nearly so; if they need to do special handling on
exit, it should be in an `atexit`-registered function, because nothing tells
libuncursed when the program is about to exit.)

### Cursor handling

    void (*setcursorsize)(int size)
    void (*positioncursor)(int y, int x)

Sets the cursor size (0 means the cursor should be hidden, 1 and 2 are visible
cursors which may or may not be implemented as different from each other, with
2 being more visible than 1), or position (with (0, 0), as always, being the
top left corner).

### Screen redraws

    void (*update)(int y, int x)
    void (*fullredraw)(void)
    void (*flush)(void)

When libuncursed wants an interface plugin to redraw all or part of the
screen, it uses these functions.  `update` tells the plugin to redraw the
character at a particular point of the screen; `fullredraw` tells it to redraw
the entire screen from scratch (which might or might not be different from
`updating` each character individually, depending on the plugin); `flush` will
be called at the end of a series of updates, and interface plugins may if they
wish postpone any actual updating until the `flush` is called (or do it
immediately, or at any time in between).  For instance, double-buffered
interfaces would swap buffers on a `flush`; the `tty` interface uses stdio
buffering for its output, which might be flushed by the stdio library when the
buffers get full, and flushes the buffers itself manually on a `flush`.

These functions do not contain any information about what the plugin should
draw; rather, it needs to obtain that information by calling back into
libuncursed and asking for it, using the `uncursed_rhook_*_at` hooks.  The
interface is allowed to update more than the requested region if it wishes
(and can use `uncursed_rhook_needsupdate` to determine whether updating a
character would be useful, if for some reason it wants to do updates out of
order), and can request the information of any part of the screen in order to
perform its update (e.g. in case if for some reason, updating one character
would disturb neighbouring characters).  The interface should call
`uncursed_rhook_updated` when it updates any character of the screen (whether
it was requested to be updated or not), so that libuncursed does not send
spurious requests to update it until the next time it actually changes.

### Input

    int (*getkeyorcodepoint)(int milliseconds)

libuncursed uses this to request one key or codepoint from the terminal;
codepoints should be returned as their Unicode value, whereas keys should be
returned as their key code plus `KEY_BIAS` (a constant declared in
`"uncursed_hooks.h"`, chosen so that there will be no clashes between keys and
codepoints).  If no keys or codepoints are available for `milliseconds`
milliseconds, the interface should instead return `KEY_SILENCE + KEY_BIAS`; if
the terminal is resized during the call to `getkeyorcodepoint` (or before the
call but after the previous call), it should call `uncursed_rhook_setsize` to
notify libuncursed of the new size, and then return `KEY_RESIZE + KEY_BIAS`.

### Graphical

All these hooks are only meaningful for graphical interface plugins; plugins
that are entirely text-based can and should set these to `NULL`.

    void (*set_faketerm_font_file)(char *filename)

Informs a graphical interface plugin of the font file it should use to do
rendering.

    void (*set_tiles_tile_file)(char *filename, int rows, int columns)

Informs a graphical interface plugin of the tile file it should associate with
tiles regions that are allocated in the future, together with the size of that
file (in tiles).

    void *(*allocate_tiles_region)(int height, int width,
       int loc_height, int loc_width, int loc_top, int loc_left)
    void (*deallocate_tiles_region)(void *region)

`allocate_tiles_region` requests a graphical interface plugin to create a new
tiles region.  A tiles region is a rectangular graphical area on which tiles
are drawn, and has a specified size (measured in tiles); it is associated with
a rectangular area of the screen, given in characters, which is the location
on which the tiles region, or subsets of it, will eventually be drawn.  (There
is no particular reason why the two areas should be the same size; if the
tiles region is smaller, it should be drawn centered on its location, with the
remainder filled with a solid color or simple pattern; if the tiles region is
larger, only a subset of it should be drawn, aiming to keep the tile
associated with the cursor location in view whenever there is a tile
associated with a cursor location.)  The tiles region itself will be treated
as an opaque pointer by libuncursed (although it will in practice most likely
be a structure containing an image and some metadata); libuncursed may request
its deallocation with `deallocate_tiles_region`.

    void (*draw_tile_at)(int tile, void *region, int y, int x)

Tells a graphical interface to draw a specified tile at the specified location
of the specified tile region.  (The tile will eventually be drawn to the
screen when `update` is called, and that region is returned by
`uncursed_rhook_region_at`.)  It is possible that a badly behaving application
may cause libuncursed to send negative or out of range values for the
coordinates `y` and `x`; interfaces should do their own bounds checks, if
necessary.

### Miscellaneous

    void (*beep)(void)

Produces an audible beep, if possible.  (There is no hook for `flash`;
uncursed synthesizes this itself out of palette changes and redraw
instructions.)

    void (*delay)(int milliseconds)

Produces a delay in the output from the interface plugin.  (If there is no
threading or pipelining going on, this can simply be implemented via a delay
function such as `usleep`.)

    void (*rawsignals)(int capture_all_keys)

The argument to this function is a boolean-like int, which specifies whether
the interface plugin should attempt to input *all* keys, including
combinations with special meanings such as Control-C (`capture_all_keys` is
1); or only the keys which are normally intended as input to applications
(`capture_all_keys` is 0).

### Future expansion

The function pointers `recordkeyorcodepoint`, `resized`, `startrecording`, and
`stoprecording` all exist in the `uncursed_hooks` structure, but should be set
to `NULL`.  (They will eventually become part of a recording/broadcast API.)

Calls from plugins into libuncursed
-----------------------------------

### Querying characters to draw

    char uncursed_rhook_cp437_at(int y, int x)
    char *uncursed_rhook_utf8_at(int y, int x)
    unsigned short uncursed_rhook_ucs2_at(int y, int x)

Queries the character at a particular screen location.  The function you would
use depends on the encoding you want; UTF-8 gives the most information, as it
can handle combining characters (the return value is a pointer to a static
array, so should be copied if it needs to remain valid past the next call to
`uncursed_rhook_utf8_at` or past the end of the hook function that calls it);
UCS-2 can handle a subset of Unicode (the Basic Multilingual Plane), and is
provided because this encoding is currently used on Windows; and for
interfaces that cannot handle Unicode, code page 437 codepoints are also
available.

    int uncursed_hook_color_at(int y, int x)
    void *uncursed_hook_region_at(int y, int x)

Queries other information about how to render a particular screen location.
The "color" uses 17-color codes (which should be converted to 8+bold by the
interface if necessary), ranging from 0 to 16; the 16th color represents the
terminal default.  They are packed into a single integer, treating it as a
bitfield; the low 5 bits are the foreground color, the next 5 bits the
background color, and the 11th (1024s) bit is set if the interface plugin
should underline the character.  The region value is non-NULL if the interface
should draw from a tile region rather than render a character; the interface
is responsible for calculating which part of the tile region to draw, based on
the bounds of the region, the location of the character, and the cursor
location (and may have to update the entire tile region in response to cursor
movements; this is part of the reason that interfaces are allowed to request
information proactively without waiting for an update to be sent).

### Update control

    void uncursed_rhook_updated(int y, int x)
    int uncursed_rhook_needsupdate(int y, int x)

Whenever an interface updates a character (whether in response to a request to
do so or not), it should notify libuncursed of the fact using the `updated`
reverse hook.  It can also query whether a character needs an update or not;
this is quite an expensive operation, and mostly useful if it allows an
interface to produce lasting improvements in the quality of its output (the
`tty` interface uses this to avoid producing output that has no effect, which
would increase the size of recordings).

### Miscellaneous

    void uncursed_rhook_setsize(int lines, int cols)

This should be called by a interface immediately before returning
`KEY_RESIZE + KEY_BIAS` from `getkeyorcodepoint`, to notify libuncursed of
what the new terminal size is.  (Before `getkeyorcodepoint` is called, the
program using libuncursed will be unaware that the size has changed;
interfaces must therefore be prepared to tolerate out-of-bounds requests.
libuncursed will tolerate out-of-bounds requests in reverse hooks for the same
reason, returning plausible data, in order to avoid a crash.)
