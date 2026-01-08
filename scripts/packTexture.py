#!/usr/bin/env python3

"""
This python script can be used to combine separate metallic and roughness texture maps into one metallicRoughness texture
because that is the format that the Renderer is using, but textures are often provided with separate texture maps

author: Felix Hommel
"""

from PIL import Image
from pathlib import Path
import argparse

def stripSuffix(filename: str, suffixes: tuple[str, ...]) -> str:
    """ Remove a suffix from a filename

    Parameters
    ----------
    filename
        The name of the file with suffix

    Returns
    -------
    str
        the name of the file excluding suffix
    """
    stemLower = filename.lower()

    for suffix in suffixes:
        if stemLower.endswith(suffix):
            return filename[: -len(suffix)]

    return filename

def main():
    # NOTE: Proces arguments
    parser = argparse.ArgumentParser(description="Combine textures of materials that have split metallic and roughness texxtures")
    parser.add_argument("--metallic", required=True, help="Path to the metallic texture")
    parser.add_argument("--roughness", required=True, help="Path to the roughness texture")
    args = parser.parse_args()

    metallicPath = Path(args.metallic)
    roughnessPath = Path(args.roughness)

    # NOTE: Read input texture images
    metallic = Image.open(metallicPath).convert("L")
    roughness = Image.open(roughnessPath).convert("L")

    if metallic.size != roughness.size:
        raise ValueError("Metallic and roughness textures must have the same size.")

    # NOTE: Generate the combined texture
    packed = Image.merge(
        "RGB",
        [
            Image.new("L", metallic.size, 0),
            roughness,
            metallic
        ]
    )

    # NOTE: Derive output path and save the combined image
    outputDir = metallicPath.parent
    baseName = stripSuffix(metallicPath.stem, ("_metallic", "-metallic", "metallic"))
    outputName = f"{baseName}-metallicRoughness{metallicPath.suffix}"
    outputPath = outputDir / outputName

    packed.save(outputPath)

if __name__ == "__main__":
    main()
