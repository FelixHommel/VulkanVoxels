from PIL import Image
from pathlib import Path
import argparse

def stripSuffix(stem: str, suffixes: tuple[str, ...]) -> str:
    stemLower = stem.lower()

    for suffix in suffixes:
        if stemLower.endswith(suffix):
            return stem[: -len(suffix)]

    return stem

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
outputName = f"{baseName}_metallicRoughness{metallicPath.suffix}"
outputPath = outputDir / outputName

packed.save(outputPath)
