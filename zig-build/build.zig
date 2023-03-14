const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {

    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});
    const isDebug = optimize == std.builtin.OptimizeMode.Debug;

    var name = if (isDebug) "wren_d" else "wren";

    const lib = b.addSharedLibrary(.{
        .name = name,
        .target = target,
        .optimize = optimize,
    });

    const flags = [_][]const u8{
        "-std=c99",
    };

    lib.linkLibC();
    lib.addCSourceFile("wren.c", &flags);
    lib.defineCMacro("WREN_API_DLLEXPORT", "1");
    if (isDebug) lib.defineCMacro("DEBUG", "1");

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    lib.install();
}
