# 4files
Here are my customizations to [4coder](http://4coder.net/). These also include customizations from other ppl
Feel free to take a look at the previews and pick out what ever you like into your own customization layer.

## Features: 
* Functions/Arrays color highlighting
* (Rust) Macro color highlighting
* Smooth cursor from ryan_fleury
  * Added Heart beat animation
* Customization to CodePeek from ryan_fleury
  * Changed rendering, now shows the file_name and which buffer is shown
  * Excludes the file you are in currently
  * Jump Backwards in history
  ![code_peek](https://github.com/Skytrias/4files/blob/master/previews/code_peek_rendering.gif)
* Indenting of '(' reworked to not be ugly (for rust) anymore
 ![paren_indenting](https://github.com/Skytrias/4files/blob/master/previews/better_paren_indenting.gif)
* ".iter()" function with '.' are indented now (for rust)
 ![dot_indenting](https://github.com/Skytrias/4files/blob/master/previews/dot_indenting.gif)
* Rust snippets
* Nord color theme
* Macro recording highlighting
 ![macro_recording](https://github.com/Skytrias/4files/blob/master/previews/macro_recording_highlight.gif)

## Minor details:
* "HACK:"" is highlighted now
* Faster scroll speed like in Alpha

## How to use (Windows):
1. Clone this repository to your `4coder/custom` folder and name it `skytrias` in my case
2. Run a x64 terminal where `cl` exists *(Build tools from VS2019)*
3. I.e. call the build `.bat` file from the root `4coder/` like `custom\skytrias\build.bat` 
    * *(if your folder has a different name, rename it or contents of the bat files)*
4. 4ed.exe has to be closed when trying to run any .bat file, otherwhise 4ed can't rebuild the `.dll` file 

## build.bat
Rebuilds the 4coder/custom_4ed.dll with all + your own `.cpp` file specified

## lex.bat
Rebuilds the lexer, calls one_time.exe, and rebuilds everything
    * The lexer speficies the words that will be highlighted in 4coder
