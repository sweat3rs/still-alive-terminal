## A rendering of Still Alive from Portal using the rudementary terminal graphics library ncurses and [miniaudio](https://github.com/mackron/miniaudio)

This was mostly just practice / fun for myself. Press 'q' to quit.

## Building:
Some **Dependencies** are included as submodules in this repo, but you will need these installed on your system:
- CMake, just the build system I use
- Ncurses with the panel library
- A Lua interpreter / C library

> If you are building on windows, installing ncurses and lua with msys2/mingw is recommended

---
**Clone** the project:<br>
~~~
>$ git clone "https://github.com/sweat3rs/still-alive-terminal" --recursive
~~~

cd into the project directory:
~~~
>$ cd still-alive-terminal
~~~

Build using cmake, for example:

~~~
>$ mkdir build

>$ cd build

>$ cmake ..

>$ make
~~~
---
If the linker cant find the ncurses panel library, you can pass the `LIBPANEL=[LIBRARY NAME]` variable definition to cmake

> Some ncurses versions on some platforms require windows to be refreshed every update. Use the `FORCE_WINDOW_REFRESH=[BOOL]` cmake variable definition to force a build with window refreshing either on or off.

## Runtime Files:

By default, all runtime dependencies live in the subdirectory named `"runtime/"` relative to where you built the project.

You can specify a new default runtime directory by passing the `RUNTIME_DIRECTORY=[DIR]` variable definition to cmake

Alternatively, you can pass the `NO_RUNTIME_DIRECTORY=true` variable definition to omit a default runtime directory.
> If NO_RUNTIME_DIRECTORY is set true, config files will be relative to the executables working directory

You can also set the runtime directory at program start using the flag:
`--runtime [DIR]`

## Config:

The config options are variables in a lua table named `config`. The default lua config file is named `config.lua` (relative to the runtime directory), but you can set it using the flag: `--config [FILE NAME]`

Config options:

- > `volume`, the song volume, a number 0-100 or 0-1 if less than 1
- > `start_delay`, the delay, in seconds, the audio file starts playing after the program starts
- > `song_file`, a string for the name of the audio file, relative to the runtime directory
- > `lyric_print_speed` / `credits_print_speed`, base print speed for the text animations, changing this alone will mess up timing
- > `icons_file` = a string for the name of the icons lua file, relative to the runtime directory. This is the file that holds the ascii art
- > `lyrics_file` / `credits_file` / `icons_timing` = strings for the names of csv files, relative to the runtime directory, with text and timing info.
- > `ignore_warnings`= a bool that sets the --ignore-warnings flag true if it is not passed as a flag.


## Terminal Compatability:
If compatiability issues with the terminal are found due to color or size, the program will exit. You can ignore these by passing the flag: `--ignore-warnings`

## Copyright Notice

All source files written by me are licensed under the GNU GPL license. Some resources used in this project like the song and its lyrics, the ascii art, and the credits belong to Valve and Jonathan Coulton. They are not mine, thus they aren't licensed by me nor under this general license.
