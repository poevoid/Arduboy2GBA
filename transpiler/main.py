import os
import sys
import argparse
import subprocess

from parser import parse_code
from mapper import map_code
from bitmap import convert_bitmaps
from audio import process_audio


SCRIPT_DIR = os.path.abspath(os.path.dirname(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))


def parse_args():
    parser = argparse.ArgumentParser(
        description="Transpile an Arduboy sketch into GBA-compatible C output."
    )

    parser.add_argument(
        "-i", "-o", "--input",
        dest="input_file",
        required=True,
        help="Path to the input .ino file, relative to the current working directory."
    )

    parser.add_argument(
        "--build-dir",
        dest="build_dir",
        default=os.path.join(PROJECT_ROOT, "build"),
        help="Directory for generated C output. Default: ../build"
    )

    parser.add_argument(
        "--asset-dir",
        dest="asset_dir",
        default=os.path.join(PROJECT_ROOT, "assets"),
        help="Directory for generated asset metadata. Default: ../assets"
    )

    parser.add_argument(
        "--run",
        action="store_true",
        help="After transpiling, invoke `make` from the project root."
    )

    parser.add_argument(
        "--make-cmd",
        default="make",
        help="Make command to run when using --run. Default: make"
    )

    return parser.parse_args()


def resolve_input_path(input_path):
    abs_path = os.path.abspath(input_path)

    if not os.path.isfile(abs_path):
        raise FileNotFoundError(f"Input file not found: {input_path}")

    if not abs_path.lower().endswith(".ino"):
        raise ValueError(f"Input file must be a .ino sketch: {input_path}")

    return abs_path


def derive_names(input_file_abs, build_dir):
    base_name = os.path.splitext(os.path.basename(input_file_abs))[0]
    build_dir_abs = os.path.abspath(build_dir)

    output_c = os.path.join(build_dir_abs, f"{base_name}.c")
    build_vars = os.path.join(build_dir_abs, "build_vars.mk")
    elf_path = os.path.join(PROJECT_ROOT, f"{base_name}.elf")
    gba_path = os.path.join(PROJECT_ROOT, f"{base_name}.gba")

    return {
        "base_name": base_name,
        "build_dir": build_dir_abs,
        "output_c": output_c,
        "build_vars": build_vars,
        "elf_path": elf_path,
        "gba_path": gba_path,
    }


def write_build_vars(build_vars_path, target_name, source_c_path):
    source_c_norm = source_c_path.replace("\\", "/")

    with open(build_vars_path, "w", encoding="utf-8") as f:
        f.write(f"TARGET := {target_name}\n")
        f.write(f"SOURCE_C := {source_c_norm}\n")


def run_make(make_cmd):
    print("=== BUILD ===")
    print("Running build command from project root:")
    print(f"  {make_cmd}")

    result = subprocess.run(
        make_cmd,
        cwd=PROJECT_ROOT,
        shell=True,
        check=False
    )

    if result.returncode != 0:
        raise RuntimeError(f"Build failed with exit code {result.returncode}")


def main():
    args = parse_args()

    input_file_abs = resolve_input_path(args.input_file)
    names = derive_names(input_file_abs, args.build_dir)

    build_dir_abs = names["build_dir"]
    asset_dir_abs = os.path.abspath(args.asset_dir)

    os.makedirs(build_dir_abs, exist_ok=True)
    os.makedirs(asset_dir_abs, exist_ok=True)

    with open(input_file_abs, "r", encoding="utf-8") as f:
        code = f.read()

    parsed = parse_code(code)

    print("=== ANALYSIS ===")
    print("Input:", input_file_abs)
    print("Project root:", PROJECT_ROOT)
    print("Libraries:", parsed.libraries)
    print("Functions:", list(parsed.functions.keys()))
    print("Bitmaps:", [b["name"] for b in parsed.bitmaps])

    # Assets / metadata
    convert_bitmaps(parsed.bitmaps, asset_dir_abs)

    # Audio normalization
    code = process_audio(parsed)
    parsed.raw_code = code

    # Main code mapping
    output_code = map_code(parsed)

    with open(names["output_c"], "w", encoding="utf-8") as f:
        f.write(output_code)

    write_build_vars(
        names["build_vars"],
        names["base_name"],
        names["output_c"]
    )

    print("=== TRANSPILATION DONE ===")
    print("Generated C:", names["output_c"])
    print("Build vars:", names["build_vars"])
    print("Expected build outputs:")
    print(" ", names["elf_path"])
    print(" ", names["gba_path"])

    if args.run:
        run_make(args.make_cmd)
        print("=== BUILD DONE ===")
        print("Generated ROM:", names["gba_path"])
    else:
        print()
        print("Next step:")
        print("  make")
        print()
        print("Or use:")
        print(f"  python main.py -o {args.input_file} --run")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
