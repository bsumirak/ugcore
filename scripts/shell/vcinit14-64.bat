REM Fixes path issues. Unfortunately, MSVC scripts don't work with quoted paths.
set path=%path:"=%

call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86_amd64
