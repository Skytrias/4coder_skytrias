# 4files
![4coder Nord](https://github.com/Skytrias/4files/blob/master/previews/nord_theme.png)

Here are my customizations to [4coder](http://4coder.net/). These also include customizations from other ppl.
Feel free to take a look at the previews and pick out what ever you like into your own customization layer.

# Features (in order of significance):
## Todo File with progress bars and markers
![todo_file_markers](https://github.com/Skytrias/4files/blob/master/previews/todo_progress_markers.gif)

## Timer / Pomomodoro Timer with breaks
![timer](https://github.com/Skytrias/4files/blob/master/previews/pomodoro_timer.gif)

## Timer paste into line end
![timer_paste](https://github.com/Skytrias/4files/blob/master/previews/timer_paste.gif)

## Snippet Autocompletion when writing
![snippet_autocompletion](https://github.com/Skytrias/4files/blob/master/previews/automatic_snippet_completion.gif)

## Functions / Arrays / struct / (Rust) macro colored highlighting
![color_highlight](https://github.com/Skytrias/4files/blob/master/previews/color_highlight.png)

## Search excludes comment lines
![exclude_search](https://github.com/Skytrias/4files/blob/master/previews/exclude_comment_search.gif)

## Smooth cursor from ryan_fleury, added Heart beat animation
![smooth_cursor](https://github.com/Skytrias/4files/blob/master/previews/smooth_cursor.gif)

## Customization to CodePeek from ryan_fleury
  * Changed rendering, now shows the file_name and which buffer is shown
  * Excludes the file you are in currently
  * Jump Backwards in history
![code_peek](https://github.com/Skytrias/4files/blob/master/previews/code_peek_rendering.gif)

## Indenting of '(' reworked to not be ugly (for rust) anymore (left new, right default)
![paren_indenting](https://github.com/Skytrias/4files/blob/master/previews/custom_indent.png)

## ".iter()" function with '.' are indented now (for rust)
![dot_indenting](https://github.com/Skytrias/4files/blob/master/previews/dot_indenting.gif)

## Rust snippets
![rust_snippets](https://github.com/Skytrias/4files/blob/master/previews/rust_snippets.gif)

## Macro recording highlighting
![macro_recording](https://github.com/Skytrias/4files/blob/master/previews/macro_recording_highlight.gif)

## (Rust Lexer) Removed character highlighting to stop this
![bad_rust_lexer](https://github.com/Skytrias/4files/blob/master/previews/bad_rust_character_lexer.gif)

## Minor Features:
* Nord color theme *see above*
* "HACK:"" is highlighted now
* Faster scroll speed like in Alpha

## How my setup works (Windows):
1. Clone this repository to your `4coder/custom` folder and name it `skytrias` in my case
2. Replace some existing `4coder_*` files with my changed versions of them in the right places
 * you might have to fix a few things if they don't exist, `4coder_cpp_lex_gen` but needs to run `lex.bat`*
3. Run a x64 terminal where `cl` exists *(Build tools from VS2019)*
4. I.e. call the build `.bat` file from the root `4coder/` like `custom\skytrias\build.bat` 
    * *(if your folder has a different name, rename it or contents of the `.bat` files)*
4. In `4coder/custom/4coder_default_hooks.cpp` disable the hooks that are overwritten in your main file -> see `skytrias.cpp` in my case
5. You will probably have to fix a few changed functions parameters because they may have been extended.
6. `4ed.exe` has to be closed when trying to run any `.bat` file, otherwhise 4ed can't rebuild the `.dll` file 

## Other
I suggest looking through each of the features, pick up what you think is cool or might be helpful to you. Extend the features you want, or send an issue / pull request.
