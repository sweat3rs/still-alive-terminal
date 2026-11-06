## A rendering of Still Alive from Portal using the rudementary terminal graphics library ncurses and [miniaudio](https://github.com/mackron/miniaudio)

## Building:
**Clone** the project:<br>
~~~
>$ git clone "https://github.com/sweat3rs0/still-alive-terminal" --recursive
~~~

cd into the project directory:
~~~
>$ cd still-alive-terminal
~~~

Build using cmake, for example:

~~~
>$ mkdir build

>$ cd build

>$ cmake .

>$ make
~~~

## Runtime Files:

By default, all runtime dependencies live in the subdirectory named `"runtime/"` relative to where you built the project.

You can specify a new default runtime directory by passing the `RUNTIME_DIRECTORY=[DIR]` flag to cmake

Alternitavly, you can pass the `NO_RUNTIME_DIRECTORY=true` flag to omit a default runtime directory.
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


## Terminal Compatability:
If compatiability issues with the terminal are found due to color or size, the program will exit. You can ignore these by passing the flag: `--ignore-warnings`

## Copyright Notice

All source files written by me are licensed under the GNU GPL license. Some resources used in this project like the song and its lyrics, the ascii art, and the credits belong to Valve and Jonathan Coulton. They are not mine, thus they aren't licensed by me nor under this general license.