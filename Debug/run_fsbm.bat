@echo off
REM 
REM This is very similar to the linux benchmarking things.
REM Only thing different from it is that this doesn't use filepath argument.
REM 

REM for large file microbenchmark
REM 1KB to 1MB
for %%i in (1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576) do fs_microbenchmark.exe large %%i 10

REM for small file microbenchmark
REM 1KB to 128KB <-- because it takes too long to make 10000 1MB files in HDD. 
for %%i in (1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072) do fs_microbenchmark.exe small %%i 10