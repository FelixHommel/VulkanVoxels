#!/usr/bin/env python3

# Little python script to compile all shaders in this project manually

import os
import subprocess
import argparse

parser = argparse.ArgumentParser(description="Compile GLSL shaders to SPIR-V")
parser.add_argument("--input", required=True, help="Input directory containing .vert/.frag files")
parser.add_argument("--output", required=True, help="Output directory for .spv files")
args = parser.parse_args()

os.makedirs(args.output, exist_ok=True)

for file in os.listdir(args.input):
    if file.endswith(".vert") or file.endswith(".frag"):
        in_path = os.path.join(args.input, file)
        out_path = os.path.join(args.output, file + ".spv")

        print(f"Compiled {file} -> {out_path}")
        result = subprocess.run(["glslangValidator", "-V", in_path, "-o", out_path])

        if result.returncode != 0:
            raise RuntimeError(f"Shader compilation failed: {file}")
