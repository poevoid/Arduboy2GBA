import os
import argparse
import subprocess

from parser import parse_code
from mapper import map_code
from optimizer import optimize_generated_c
from include_loader import expand_local_includes


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-in", "--input", required=True, help="Input .ino file")
    ap.add_argument("--run", action="store_true", help="Build after transpiling")
    ap.add_argument(
        "--time-scale",
        type=float,
        default=1.0,
        help="Global timing scale. 1.0 = normal, 0.5 = 2x faster, 2.0 = slower",
    )
    ap.add_argument(
        "--out",
        default=None,
        help="Directory for generated .c, .elf, and .gba outputs. Defaults to the input file's folder.",
    )
    ap.add_argument(
        "--output-dir",
        dest="out_compat",
        default=None,
        help="Deprecated alias for --out.",
    )
    args = ap.parse_args()

    input_file = os.path.abspath(args.input)
    project_root = os.getcwd()

    if not os.path.exists(input_file):
        raise FileNotFoundError(f"Input file not found: {input_file}")

    base_name = os.path.splitext(os.path.basename(input_file))[0]
    time_scale = args.time_scale
    if time_scale <= 0.0:
        time_scale = 1.0

    output_dir_arg = args.out if args.out is not None else args.out_compat
    if output_dir_arg is None:
        output_dir = os.path.dirname(input_file)
    else:
        output_dir = os.path.abspath(output_dir_arg)

    os.makedirs(output_dir, exist_ok=True)

    build_dir = os.path.join(output_dir, "build")
    os.makedirs(build_dir, exist_ok=True)

    print("=== ANALYSIS ===")
    print(f"Input: {input_file}")
    print(f"Project root: {project_root}")
    print(f"Output dir: {output_dir}")

    source = expand_local_includes(input_file)
    parsed = parse_code(source)

    print(f"Libraries: {{'arduboy2': True, 'arduboy': False, 'playtune': False}}")
    print(f"Functions: {parsed.functions}")
    print(f"Bitmaps: {list(parsed.bitmaps.keys())}")

    mapped_code = map_code(parsed)
    optimized_code = optimize_generated_c(mapped_code, game_name=base_name)

    out_c = os.path.join(build_dir, f"{base_name}.c")
    with open(out_c, "w", encoding="utf-8") as f:
        f.write(optimized_code)

    out_elf = os.path.join(output_dir, f"{base_name}.elf")
    out_gba = os.path.join(output_dir, f"{base_name}.gba")

    build_vars = os.path.join(project_root, "build", "build_vars.mk")
    os.makedirs(os.path.dirname(build_vars), exist_ok=True)

    game_profile = "MYBL_AB" if base_name == "MYBL_AB" else ""

    with open(build_vars, "w", encoding="utf-8") as f:
        f.write(f"TARGET={base_name}\n")
        f.write(f"SOURCE_C={out_c}\n")
        f.write(f"OUTPUT_ELF={out_elf}\n")
        f.write(f"OUTPUT_GBA={out_gba}\n")
        f.write(f"TIME_SCALE={time_scale}\n")
        f.write(f"GAME_PROFILE={game_profile}\n")

    print("=== TRANSPILATION DONE ===")
    print(f"Generated C: {out_c}")
    print(f"Build vars: {build_vars}")
    print("Expected build outputs:")
    print(f"  {out_elf}")
    print(f"  {out_gba}")
    print(f"Time scale: {time_scale}")
    if game_profile:
        print(f"Game-specific optimizer profile: {game_profile}")

    if args.run:
        print("=== BUILD ===")
        print("Running build command from project root:")
        print("  make")
        rc = subprocess.call(["make"], cwd=project_root)
        if rc != 0:
            print(f"ERROR: Build failed with exit code {rc}")
            raise SystemExit(rc)


if __name__ == "__main__":
    main()
