#!/usr/bin/env python3

"""
Script used in the texture normalization process to unify the texture naming convention for the project

author: Felix Hommel
"""

import argparse
import re
import sys
from pathlib import Path
from collections import defaultdict, Counter

# Extensions of images that are considered to be searched
IMAGE_EXTENSIONS_PATTERNS = { ".png", ".jpg", ".jpeg", ".ktx" }

# What pattern in a filename hints to what type of texture map it is
MAP_PATTERNS = {
    "albedo": [
        r"albedo", r"(base)?[_\-]?color", r"diff(use)?"
    ],
    "normal": [
        r"[_\-]?n(ormal)?", r"[_\-]?nrm", r"[_\-]?normal[_\-]?(ogl|dx)"
    ],
    "metallic": [
        r"[_\-]?metallic", r"\bmetal\b"
    ],
    "roughness": [
        r"[_\-]?rough(ness)?", r"\brough\b"
    ],
    "metallicRoughness": [
        r"[_\-]?metallic[_\-]?roughness"
    ],
    "ao": [
        r"[_\-]?ao", r"\bao\b", r"occlusion", r"ambient"
    ],
    "emissive": [
        r"emissive", r"emission", r"glow"
    ],
    "height": [
        r"[_\-]?height", r"[_\-]?h"
    ]
}

BASE_STRIP_PATTERNS = [
    r"albedo", r"(base)?[_\-]?color", r"diff(use)?",
    r"[_\-]?n(ormal)?", r"nrm",
    r"[_\-]?metallic", r"[_\-]?rough(ness)?", r"[_\-]?metallic[_\-]?roughness",
    r"[_\-]?ao", r"occlusion", r"ambient",
    r"emissive", r"emission", r"glow",
    r"(ogl|dx)",
    r"[_\-]?h(eight)?"
]

BASE_STRIP_REGEX = re.compile(r"[_\-\.]?(%s)[_\-\.]?" % "|".join(BASE_STRIP_PATTERNS), re.IGNORECASE)
SEPARATOR_REGEX = re.compile(r"[_\-\.]+")
PREVIEW_IMAGE_REGEX = re.compile(r"[_\-]?preview[_\-]?", re.IGNORECASE)

def is_image(path: Path) -> bool:
    """ Determine if the path leads to an image that is considered to normalize

    Parameters
    ----------
    path : Path
        The filepath that is checked

    Returns
    -------
    bool
        true if the path leads to an image that is checked, false if not
    """
    return path.suffix.lower() in IMAGE_EXTENSIONS_PATTERNS

def is_preview(filename: str) -> bool:
    """ Check if the file is a texture preview file

    Parameters
    ----------
    filename : str
        the name of the file

    Returns
    -------
    bool
        true if it is a preview file, false if not
    """
    return PREVIEW_IMAGE_REGEX.search(filename) is not None

def normalize_name(name: str) -> str:
    """ Normalize the filename with the provided regex

    Parameters
    -----------
    name : str
        the current filename

    Returns
    -------
    str
        the new filename
    """
    name = name.lower()
    name = BASE_STRIP_REGEX.sub("", name)
    name = SEPARATOR_REGEX.sub("-", name)
    return name.strip("-")

def classify_map(filename: str) -> tuple[str | None, int]:
    """ Analyze and determine what type of texture map it is

    Parameters
    ----------
    filename : str
        The filepath of the texture

    Returns
    -------
    tuple
        (map_type, confidence_score)
    """
    name = filename.lower()
    scores = Counter()

    for map_type, patterns in MAP_PATTERNS.items():
        for p in patterns:
            if re.search(p, name):
                scores[map_type] += 1

    if not scores:
        return None, 0

    return scores.most_common(1)[0]

def scan_directory(directory: Path) -> dict[str, list[str]]:
    """ Search a directory for valid textures

    Parameters
    ----------
    directory : Path
        path to the directory to search for textures

    Returns
    -------
    dict(str, list(str))
        A dictionary with normalized filename to all files that are represented by this file
    """
    materials = defaultdict(list)

    for file in directory.iterdir():
        if not file.is_file() or not is_image(file) or is_preview(file.stem):
            continue;

        base = normalize_name(file.stem)
        materials[base].append(file)

    return materials

def build_rename_plan(materials):
    """ Classify each of the candidate texture files and find a rename strategy based on the classification

    Parameters
    ----------
    materials
    """
    plan = []

    for material, files in materials.items():
        classified = defaultdict(list)

        for f in files:
            map_type, score = classify_map(f.name)
            classified[(map_type, score)].append(f)

        for (map_type, score), group in classified.items():
            if map_type is None:
                continue;

            if len(group) > 1:
                continue;

            src = group[0]
            dst = src.with_name(f"{material}-{map_type}{src.suffix}")

            if src.name != dst.name:
                plan.append((src, dst, score))

    return plan

def print_plan(plan):
    """ Print the proposed plan to the console for manual verification

    Parameters
    ----------
    tuple(str, str, int)
        tuple of old name, new name, confidence in correct classification of the file
    """
    if not plan:
        print("No changes detected")
        return

    print("\nProposed renames:\n")
    for src, dst, score in plan:
        print(f"[{score}] {src.name} -> {dst.name}")
    print()

def apply_plan(plan):
    """ Use the provided plan and rename the texture maps accordingly

    Parameters
    ----------
    tuple(str, str, int)
        tuple of old name, new name, confidence in correct classification of the file
    """
    for src, dst, _ in plan:
        if dst.exists():
            print(f"SKIP (exists): {dst.name}")
            continue

        src.rename(dst)

def main():
    parser = argparse.ArgumentParser(description="PBR texture renaming and grouping tool")
    parser.add_argument("directory", type=Path)
    parser.add_argument("--apply", action="store_true", help="Apply changes")
    args = parser.parse_args()

    if not args.directory.is_dir():
        print("Error: directory does not exist.")
        sys.exit(1)

    materials = scan_directory(args.directory)
    plan = build_rename_plan(materials)

    if len(plan) == 0:
        print("No changes are needed.")
        sys.exit(0)

    print_plan(plan)

    if args.apply:
        confirm = input("Apply these changes? [y/N]: ").strip().lower()
        if confirm == "y":
            apply_plan(plan)
            print("Done.")
        else:
            print("Aborted.")
    else:
        print("Dry-run only. Use --apply to rename files.")

if __name__ == "__main__":
    main()
