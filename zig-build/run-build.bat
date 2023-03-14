REM https://github.com/ziglang/zig/blob/master/lib/std/target.zig
DEL wren.c
python ../util/generate_amalgamation.py >> wren.c
XCOPY "wren.c" ".\zig-out\lib\" /Y
zig build -Dtarget=x86_64-windows -Doptimize=Debug
zig build -Dtarget=x86_64-linux -Doptimize=Debug
zig build -Dtarget=x86_64-macos -Doptimize=Debug
zig build -Dtarget=x86_64-windows -Doptimize=ReleaseSafe
zig build -Dtarget=x86_64-linux -Doptimize=ReleaseSafe
zig build -Dtarget=x86_64-macos -Doptimize=ReleaseSafe
PAUSE