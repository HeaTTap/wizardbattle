#!/bin/bash
# Download free CC0 game assets for WizardBattle
set -e

ASSET_DIR="$(cd "$(dirname "$0")/.." && pwd)/assets"
MODELS="$ASSET_DIR/models"
TEXTURES="$ASSET_DIR/textures"
SOUNDS="$ASSET_DIR/sounds"

mkdir -p "$MODELS" "$TEXTURES" "$SOUNDS"

echo "=== Downloading WizardBattle assets ==="

BASE="https://raw.githubusercontent.com/quaternius/Ultimate-Stylized-3D-Character-Pack/main/Models"

echo "Downloading wizard model..."
curl -sL "$BASE/Mage/Mage.obj" -o "$MODELS/wizard.obj" || echo "Wizard: using placeholder"

echo "Downloading goblin model..."
curl -sL "$BASE/Goblin/Goblin.obj" -o "$MODELS/goblin.obj" || echo "Goblin: using placeholder"

echo "Downloading ogre model..."
curl -sL "$BASE/Cyclops/Cyclops.obj" -o "$MODELS/ogre.obj" || echo "Ogre: using placeholder"

echo "Downloading archer model..."
curl -sL "$BASE/Elf/Elf.obj" -o "$MODELS/archer.obj" || echo "Archer: using placeholder"

echo "Downloading dragon model..."
curl -sL "$BASE/Dragon/Dragon.obj" -o "$MODELS/dragon.obj" || echo "Dragon: using placeholder"

echo "Downloading tree model..."
curl -sL "https://raw.githubusercontent.com/quaternius/Low-Poly-Forest-Pack/main/Models/Tree1.obj" -o "$MODELS/tree.obj" || echo "Tree: using placeholder"

echo "Downloading rock model..."
curl -sL "https://raw.githubusercontent.com/quaternius/Low-Poly-Forest-Pack/main/Models/Rock1.obj" -o "$MODELS/rock.obj" || echo "Rock: using placeholder"

echo "=== Download complete ==="
echo "Models in: $MODELS"
echo "Total size: $(du -sh "$ASSET_DIR" 2>/dev/null | cut -f1)"
