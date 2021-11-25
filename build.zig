const std = @import("std");
const builtin = @import("builtin");

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    const lib = b.addSharedLibrary("watcher", null, b.version(0, 0, 1));
    lib.setTarget(target);
    lib.setBuildMode(mode);
    lib.force_pic = true;
    lib.linkLibCpp();
    lib.addIncludeDir("./include/");
    lib.addCSourceFiles(&.{
        "src/capi.cc",
        "src/Watcher.cc",
        "src/Options.cc",
        "src/DirTree.cc",
    }, &.{});

    const tag = target.getOsTag();
    if (tag == std.Target.Os.Tag.windows) {
        std.debug.print("building for windows\n", .{});
        lib.addCSourceFiles(&.{
            "src/Backend.cc",
            "src/shared/BruteForceBackend.cc",
            "src/windows/WindowsBackend.cc",
            "src/windows/win_utils.cc",
        }, &.{
            "-DWINDOWS",
            "-DBRUTE_FORCE",
        });
    } else if (tag == std.Target.Os.Tag.macos) {
        std.debug.print("building for macos\n", .{});
        lib.addCSourceFiles(&.{
            "src/Backend.cc",
            "src/shared/BruteForceBackend.cc",
            "src/macos/FSEventsBackend.cc",
        }, &.{
            "-DFS_EVENTS",
            "-DBRUTE_FORCE",
        });
    } else if (tag == std.Target.Os.Tag.linux) {
        std.debug.print("building for linux\n", .{});
        lib.addCSourceFiles(&.{
            "src/Backend.cc",
            "src/shared/BruteForceBackend.cc",
            "src/linux/InotifyBackend.cc",
            "src/unix/legacy.cc",
        }, &.{
            "-DINOTIFY",
            "-DBRUTE_FORCE",
        });
    }

    lib.install();
}
