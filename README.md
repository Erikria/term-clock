# Term-Clock
## Description
`term-clock` displays a simple digital clock on the terminal.
## Usage
```
term-clock [-iuvsScbtrahDBxn] [-C [0-7]] [-f format] [-d delay] [-a nsdelay] [-T term]
```
## Options
```
    -s            Show seconds
    -S            Screensaver mode
    -x            Show box
    -c            Set the clock at the center of the terminal
    -C [0-7]      Set the clock color
    -b            Use bold colors
    -t            Set the hour in 12h format
    -u            Use UTC time
    -T term       Display the clock on the specified terminal
    -r            Do rebound the clock
    -f format     Set the date format
    -n            Don't quit on keypress
    -v            Show tty-clock version
    -i            Show some info about tty-clock
    -h            Show this page
    -D            Hide date
    -B            Enable blinking colon
    -d delay      Set the delay between two redraws of the clock. Default 1s.
    -a nsdelay    Additional delay between two redraws in nanoseconds. Default 0ns.
```
