#!/usr/bin/env python3

import argparse
import subprocess
import platform
import shutil
import os


def configure_cmake(build_type: str, target_platform: str) -> None:
    cmake_flags = [
        "cmake",
        "-S", ".",
        "-B", "build/",
        "-GNinja",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=1",
        f"-DCMAKE_BUILD_TYPE={build_type}",
        f"-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/{
            target_platform.lower()}.cmake.in"
    ]

    print("Configuring cmake...")
    subprocess.run(cmake_flags, check=True)


def build_engine() -> None:
    build_path = "build/"

    print("Building engine...")
    subprocess.run(["cmake", "--build", build_path], check=True)


def run_tests() -> None:
    test_path = "bin/tests/glitch-tests"

    if os.path.exists(test_path):
        print("Running test executable...")

        subprocess.run([test_path], check=True)
    else:
        print("Test application could not be found.")


def run_testbed() -> None:
    testbed_path = "bin/glitch-testbed"

    if os.path.exists(testbed_path):
        print("Running testbed executable...")

        subprocess.run([testbed_path], check=True)
    else:
        print("Testbed application could not be found.")


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
    supported_systems = [
        "Linux",
        "Windows"
    ]

    system = platform.system()
    if system not in supported_systems:
        print("Unsupported platform!")
        return

    parser = argparse.ArgumentParser(
        prog="Glitch Builder",
        description="Build script for glitch engine."
    )

    parser.add_argument("action",
                        choices=["build", "clean", "test", "testbed"],
                        help="Action to perform"
                        )

    parser.add_argument("--config",
                        choices=[
                            "Debug",
                            "Release",
                            "RelWithDebInfo",
                            "MinSizeRel"
                        ],
                        default="Debug",
                        help="Build type"
                        )
    parser.add_argument("--platform",
                        choices=[
                            "Windows",
                            "Linux"
                        ],
                        default=system,
                        help="Target platform to build"
                        )

    args = parser.parse_args()

    try:
        if args.action == "build":
            configure_cmake(args.config, args.platform)
            build_engine()
        elif args.action == "test":
            run_tests()
        elif args.action == "testbed":
            run_testbed()
        elif args.action == "clean":
            clean()
    except subprocess.CalledProcessError as e:
        print(f"Application exited with code: {e.returncode}.")
        exit(e.returncode)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass