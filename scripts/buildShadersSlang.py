#!/usr/bin/env python3

"""
Python script to compile Slang shaders into SPIR-V binaries

author: Felix Hommel
"""

import os
import subprocess
import argparse

parser = argparse.ArgumentParser(description="Compile Slang shaders to SPIR-V")
parser.add_argument("--input", required=True, help="Input directory containing .slag files")
parser.add_argument("--output", required=True, help="Output directory for .spv files")
args = parser.parse_args()

os.makedirs(args.output, exist_ok=True)

for file in os.listdir(args.input):
    if file.endswith(".slang"):
        if "Vert" in file:
            stage = "vertex"
        elif "Frag" in file:
            stage = "fragment"
        else:
            print(f"Skipping {file}: Unknown shader stage")
            continue

        in_path = os.path.join(args.input, file)
        out_path = os.path.join(args.output, file.removesuffix(".slang") + ".spv")

        result = subprocess.run(["slangc", "-stage", stage, "-target", "spirv", "-entry", "main", "-o", out_path, in_path])

        if result.returncode != 0:
            raise RuntimeError(f"Shader compilation failed: {file}")

        print(f"Compiled {file} -> {out_path}")
    else:
        print(f"Skipping {file}: Not a slang shader")
