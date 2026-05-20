Import("env")
import os

def merge_bin(source, target, env):
    firmware_path = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")
    bootloader_path = os.path.join(env.subst("$BUILD_DIR"), "bootloader.bin")
    partitions_path = os.path.join(env.subst("$BUILD_DIR"), "partitions.bin")
    filesystem_path = os.path.join(env.subst("$BUILD_DIR"), "littlefs.bin")
    output_path = os.path.join(env.subst("$PROJECT_DIR"), "/dist/firmware.bin")
    
    cmd = [
        "esptool.py",
        "--chip", "esp32",
        "merge_bin",
        "-o", output_path,
        "0x1000", bootloader_path,
        "0x8000", partitions_path,
        "0x10000", firmware_path,
        "0x3D0000", filesystem_path
    ]
    env.Execute(" ".join(cmd))

env.AddPostAction("buildprog", merge_bin)