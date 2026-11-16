@echo off
echo Compiling Interpreter tests...
set CFLAGS=-std=c11 -Wall -Wextra -Werror -O2 -DWYRM_NO_CURL -I../../../wyrm/lib
set RUNTIME=../../../wyrm/lib

gcc %CFLAGS% -c %RUNTIME%/wyrm_core.c -o wyrm_core.o
gcc %CFLAGS% -c %RUNTIME%/wyrm_arena.c -o wyrm_arena.o
gcc %CFLAGS% -c %RUNTIME%/wyrm_str.c -o wyrm_str.o
gcc %CFLAGS% -c %RUNTIME%/wyrm_ffi.c -o wyrm_ffi.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_json.c -o wyrm_std_json.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_yaml.c -o wyrm_std_yaml.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_collections.c -o wyrm_std_collections.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_random.c -o wyrm_std_random.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_time.c -o wyrm_std_time.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_os.c -o wyrm_std_os.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_fs.c -o wyrm_std_fs.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_path.c -o wyrm_std_path.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_io.c -o wyrm_std_io.o
gcc %CFLAGS% -c %RUNTIME%/stdlib/wyrm_std_http.c -o wyrm_std_http.o

g++ -std=c++20 -Wall -Wextra -Werror -O2 -DWYRM_NO_CURL test_interpreter.cpp ../interpreter.cpp ../builtins.cpp ../stdlib_setup.cpp ../../parser/parser.cpp ../../lexer/lexer.cpp wyrm_core.o wyrm_arena.o wyrm_str.o wyrm_ffi.o wyrm_std_json.o wyrm_std_yaml.o wyrm_std_collections.o wyrm_std_random.o wyrm_std_time.o wyrm_std_os.o wyrm_std_fs.o wyrm_std_path.o wyrm_std_io.o wyrm_std_http.o -o test_interpreter.exe
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
