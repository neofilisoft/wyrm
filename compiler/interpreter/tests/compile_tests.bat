@echo off
echo Compiling Interpreter tests...
gcc -std=c11 -Wall -Wextra -Werror -O2 -c ../../../wyrm/lib/wyrm_core.c -o wyrm_core.o
gcc -std=c11 -Wall -Wextra -Werror -O2 -c ../../../wyrm/lib/wyrm_arena.c -o wyrm_arena.o
gcc -std=c11 -Wall -Wextra -Werror -O2 -c ../../../wyrm/lib/wyrm_str.c -o wyrm_str.o
g++ -std=c++20 -Wall -Wextra -Werror -O2 test_interpreter.cpp ../interpreter.cpp ../builtins.cpp ../../parser/parser.cpp ../../lexer/lexer.cpp wyrm_core.o wyrm_arena.o wyrm_str.o -o test_interpreter.exe
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED!
    exit /b %ERRORLEVEL%
)
echo Compilation successful. Running tests...
.\test_interpreter.exe
if %ERRORLEVEL% neq 0 (
    echo Test execution FAILED!
    exit /b %ERRORLEVEL%
)
echo All tests passed.
exit /b 0
