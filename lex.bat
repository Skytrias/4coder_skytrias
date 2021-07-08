@echo off
call custom\bin\build_one_time.bat custom\languages\4coder_cpp_lexer_gen.cpp
call one_time.exe
call release.bat