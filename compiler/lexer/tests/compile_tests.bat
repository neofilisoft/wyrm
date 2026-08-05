@echo off
echo Compiling Lexer tests...
g++ -std=c++20 -Wall -Wextra -Werror -O2 test_lexer.cpp ../lexer.cpp -o test_lexer.exe
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED!
    exit /b %ERRORLEVEL%
)
echo Compilation successful. Running tests...
.\test_lexer.exe
if %ERRORLEVEL% neq 0 (
    echo Test execution FAILED!
    exit /b %ERRORLEVEL%
)
echo All tests passed.
exit /b 0
