# 4Coder Customization Layer for [Odin](http://odin-lang.org/)
4Coder Customization Layer that I use daily for **Odin**
Customizations are kept minimal for my / your sake

## Odin Features
* Keywords + Highlight
* Code Indexing (*proc, structs, enums, unions, constants*)
* Goto jumping since Code Indexing works
* Error / Lister Jumping

## How to try out :tada:
Windows: Replace your `custom_4coder.dll` with mine
Linux: Replace your `custom_4coder.so` with mine

## How to build
Replace all files to their corresponding locations, i.e. `custom` folder files belong to custom etc.
To get **odin** keyword lexing you have to rebuild the lexer with my custom language file
The scripts have to live in the root `4coder` folder

Release script rebuilds code 
Lex script rebuilds the lexer and calls release script

Windows: `release.bat` | `lex.bat`
Linux: `release.sh` | `lex.sh`

## Themes
* Solarized Dark
* Nord
* Horizon

## Old 
You can browse the repo history for other extensions i used to use
