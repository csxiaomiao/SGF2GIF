# SGF2GIF
Project SGF2GIF is a small tool to convert *.SGF ([Smart Game Format](https://en.wikipedia.org/wiki/Smart_Game_Format) for Go programs) to animated GIF images.

This program is written in C++ using the Microsoft Windows API.  It can be compiled and linked in the Microsoft Visual Studio IDEs.  If attempting to cross-compile on Unix for WINE with `i686-w64-mingw32-g++` you will need the file `res/resource.h` (this file is not currently included in the repository, so you'll probably need VC++ to generate it).

## How to run this program

* In Windows Explorer, drag an `SGF` file and drop it on the compiled `sgf2gif.exe`, and the corresponding `GIF` file will be generated.

* Alternatively, you may run `sgf2gif.exe` directly and choose the input and output files.

## Parameters in the options window

Playing Speed: an unsigned integer, smaller is faster, default 50
    
Displaying the number of stones: an unsigned integer, show the last `N` plays (can be set to 0)
    
Cut of the play sequence: undocumented
   
Size of Stone: 15 - 50 (pixels)
 
