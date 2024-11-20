#!/usr/bin/env python3

import argparse
import subprocess
import platform
import shutil
import os

from pathlib import Path


def configure_cmake(build_type: str, target_platform: str) -> None:
    cmake_flags = [
        "cmake",
        "-S",
        ".",
        "-B",
        "build/",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=1",
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]

    print("Configuring cmake...")
    subprocess.run(cmake_flags, check=True)


def export_headers() -> None:
    exclude_directories = [
        "platform",
    ]

    src_dir = Path("./src")
    dest_dir = Path("./bin/include")

    if dest_dir.exists():
        shutil.rmtree(dest_dir)
    else:
        os.makedirs(dest_dir)

    for path in src_dir.rglob("*.h"):
        if any(exclude in path.parts for exclude in exclude_directories):
            continue

        # Create corresponding directory structure in bin/include
        relative_path = path.relative_to(src_dir)
        dest_path = dest_dir / relative_path

        dest_path.parent.mkdir(parents=True, exist_ok=True)

        # Copy the header file to the destination
        shutil.copy2(path, dest_path)

    # Copy GLM headers
    glm_dest = dest_dir / "glm"
    if glm_dest.exists():
        shutil.rmtree(glm_dest)

    shutil.copytree("./third_party/glm/glm", glm_dest)

    # Copy stb headers
    stb_dest = dest_dir / "stb"
    if stb_dest.exists():
        shutil.rmtree(stb_dest)

    shutil.copytree("./third_party/stb", stb_dest)

    # Copy ImGui headers
    imgui_dest = dest_dir / "imgui"
    if imgui_dest.exists():
        shutil.rmtree(imgui_dest)

    imgui_dest.mkdir(parents=True, exist_ok=True)

    # Copy individual header files from the source directories
    source_files = [
        "./third_party/imgui/*.h",
        "./third_party/imgui/misc/cpp/imgui_stdlib.h",
    ]

    for pattern in source_files:
        for file in Path().glob(pattern):
            shutil.copy(file, imgui_dest)


def export_libs() -> None:
    exclude_libs = ["gl-vulkan-backend"]

    src_dir = Path("./build/lib")
    dest_dir = Path("./bin/lib")

    if dest_dir.exists():
        shutil.rmtree(dest_dir)
    else:
        os.makedirs(dest_dir)

    for path in src_dir.rglob("*gl-*.*"):
        if any(exclude in part for exclude in exclude_libs for part in path.parts):
            continue

        # Create corresponding directory structure in bin/lib
        relative_path = path.relative_to(src_dir)
        dest_path = dest_dir / relative_path

        dest_path.parent.mkdir(parents=True, exist_ok=True)

        shutil.copy2(path, dest_path)


def build_engine() -> None:
    build_path = "build/"

    print("Building engine...")
    result = subprocess.run(["cmake", "--build", build_path], check=True)

    if result.returncode == 0:
        export_headers()
        export_libs()


def run_tests() -> None:
    test_path = "bin/tests/glitch-tests"

    if os.path.exists(test_path):
        print("Running test executable...")

        subprocess.run([test_path], check=True)
    else:
        print("Test application could not be found.")


def clean() -> None:
    build_path = "build/"
    bin_path = "bin/"
    cache_path = ".glitch/"

    print("Cleaning...")

    if os.path.exists(build_path):
        shutil.rmtree(build_path)

    if os.path.exists(bin_path):
        shutil.rmtree(bin_path)

    if os.path.exists(cache_path):
        shutil.rmtree(cache_path)


def main() -> None:
    supported_systems = ["Linux", "Windows"]

    system = platform.system()
    if system not in supported_systems:
        print("Unsupported platform!")
        return

    parser = argparse.ArgumentParser(
        prog="Glitch Builder", description="Build script for glitch engine."
    )

    parser.add_argument(
        "action", choices=["build", "clean", "test"], help="Action to perform"
    )

    parser.add_argument(
        "--config",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
        default="Debug",
        help="Build type",
    )
    parser.add_argument(
        "--platform",
        choices=["Windows", "Linux"],
        default=system,
        help="Target platform to build",
    )

    args = parser.parse_args()

    try:
        if args.action == "build":
            configure_cmake(args.config, args.platform)
            build_engine()
        elif args.action == "test":
            run_tests()
        elif args.action == "clean":
            clean()
    except Exception as e:
        print(f"Application exited with code: {e.returncode}.")
        exit(e.returncode)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
