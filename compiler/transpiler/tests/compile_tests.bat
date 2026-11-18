@echo off
echo Compiling Transpiler tests...
set RUNTIME=^
 ..\..\..\wyrm\lib\wyrm_core.o ^
 ..\..\..\wyrm\lib\wyrm_arena.o ^
 ..\..\..\wyrm\lib\wyrm_str.o ^
 ..\..\..\wyrm\lib\wyrm_ffi.o ^
 ..\..\..\wyrm\lib\wyrm_std_json.o ^
 ..\..\..\wyrm\lib\wyrm_std_yaml.o ^
 ..\..\..\wyrm\lib\wyrm_std_collections.o ^
 ..\..\..\wyrm\lib\wyrm_std_random.o ^
 ..\..\..\wyrm\lib\wyrm_std_time.o ^
 ..\..\..\wyrm\lib\wyrm_std_os.o ^
 ..\..\..\wyrm\lib\wyrm_std_fs.o ^
 ..\..\..\wyrm\lib\wyrm_std_path.o ^
 ..\..\..\wyrm\lib\wyrm_std_io.o ^
 ..\..\..\wyrm\lib\wyrm_std_http.o

g++ -std=c++20 -Wall -Wextra -Werror -O2 test_transpiler.cpp ..\transpiler.cpp ..\..\lexer\lexer.cpp ..\..\parser\parser.cpp ..\..\interpreter\interpreter.cpp ..\..\interpreter\builtins.cpp ..\..\interpreter\stdlib_setup.cpp %RUNTIME% -o test_transpiler.exe
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED!
    exit /b %ERRORLEVEL%
)
echo Running transpiler tests (scratch dir: %TEMP%\wyrm_transpiler_tests)...
.\test_transpiler.exe "%TEMP%\wyrm_transpiler_tests" ..\..\..
if %ERRORLEVEL% neq 0 (
    echo Test execution FAILED!
    exit /b %ERRORLEVEL%
)

g++ -std=c++20 -Wall -Wextra -Werror -O2 test_llvm_short_circuit.cpp -o test_llvm_short_circuit.exe
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED!
    exit /b %ERRORLEVEL%
)
echo Running LLVM structural test...
.\test_llvm_short_circuit.exe %1
if %ERRORLEVEL% neq 0 (
    echo Test execution FAILED!
    exit /b %ERRORLEVEL%
)
echo All tests passed.
exit /b 0
