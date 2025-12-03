# DOOMERS - Required 3D Models & Assets

## Overview
This document lists all 3D models needed for the DOOMERS game.
**Now supports: FBX, GLTF, OBJ** (with Assimp library)

**Recommended: FBX format** for animated characters (from Mixamo)

---

## Model Loading

### Animated Models (FBX - Characters)
```cpp
// Load animated FBX model
AnimatedModel* soldier = LoadAnimatedModel("assets/player/soldier.fbx");

// Update animation each frame
soldier->update(deltaTime);

// Change animation
soldier->setAnimationByName("Rifle Walk");

// Draw
soldier->draw();
```

### Static Models (OBJ - Props)
```cpp
// Load static OBJ model
Mesh* crate = LoadMesh("assets/crate/crate.obj");

// Draw
crate->draw();
```

---

## LEVEL 1: Abandoned Research Facility (Indoor Lab)

### Player & Enemies
| Model | Description | Suggested Sketchfab Search |
|-------|-------------|---------------------------|
| **Soldier** | Armed player character with gun | "sci-fi soldier" or "military soldier" |
| **Zombie** | Basic enemy, slow but dangerous | "zombie character" or "undead" |
| **Demon** | Fast demon enemy | "demon monster" or "hell creature" |

### Weapons
| Model | Description | Notes |
|-------|-------------|-------|
| **Sci-Fi Rifle** | Main weapon with flashlight | "sci-fi rifle" or "assault rifle" |

### Environment - Structures
| Model | Description | Suggested Sketchfab Search |
|-------|-------------|---------------------------|
| **Metal Crate** | For cover and parkour | "sci-fi crate" or "metal box" |
| **Sci-Fi Box** | Smaller obstacle | "cargo container" |
| **Security Door** | Sliding lab doors | "sci-fi door" or "blast door" |
| **Gate Frame** | Door frames | "door frame industrial" |
| **Lab Corridor** | Modular corridor pieces | "sci-fi corridor" |

### Collectibles
| Model | Description | Suggested Sketchfab Search |
|-------|-------------|---------------------------|
| **Health Pack** | Restores player health | "health pack" or "medkit" |
| **Ammo Box** | Restores ammunition | "ammo box" or "ammo crate" |
| **Keycard** | Opens security doors | "keycard" or "access card" |

### Level Goal
| Model | Description | Suggested Sketchfab Search |
|-------|-------------|---------------------------|
| **Lab Core/Reactor** | End-of-level objective | "reactor core" or "portal device" |

---

## LEVEL 2: Hell Arena (Outdoor)

### Enemies
| Model | Description | Suggested Sketchfab Search |
|-------|-------------|---------------------------|
| **Boss Demon** | Large boss enemy | "demon boss" or "hell guardian" |
| **Heavy Soldier** | Armored enemy variant | "heavy soldier" or "armored enemy" |

### Environment - Structures
| Model | Description | Suggested Sketchfab Search |
|-------|-------------|---------------------------|
| **AC Unit** | Rooftop obstacle | "air conditioning unit" |
| **Pipes** | Industrial pipes | "industrial pipes" |
| **Chimney** | Rooftop obstacle | "chimney industrial" |
| **Antenna** | Communication tower | "radio antenna" |
| **Floating Platform** | Parkour platforms | "metal platform" |
| **Broken Bridge** | Parkour element | "destroyed bridge piece" |

### Level Goal
| Model | Description | Suggested Sketchfab Search |
|-------|-------------|---------------------------|
| **Obelisk/Portal** | Final objective | "glowing obelisk" or "hell portal" |

---

## Current Assets in `assets/` Folder

The project already contains some assets that may be usable:

```
assets/
├── AR/                          - Gun model (potential weapon)
├── gart130-crate/               - Crate model ✓
├── health-pack/                 - Health pack ✓
├── military-man-army-man-soldier/ - Player character ✓
├── sci-fi-ammo-box/             - Ammo box ✓
├── sci-fi-corridor-texturing-challenge/ - Corridor pieces ✓
├── sci-fi-gate/                 - Gate/door model ✓
├── Soldier/                     - Alternative soldier model
└── zombie/                      - Zombie enemy ✓
```

---

## Model Loading

Models are loaded using the OBJ loader in `Engine/ResourceManager.hpp`.

### Usage Example:
```cpp
// Load a model
Mesh* zombieMesh = ResourceManager::loadOBJ("assets/zombie/source/obj/obj/Zombie.obj");

// Draw the model
if (zombieMesh) {
    zombieMesh->draw();
}
```

---

## Animation Notes

Since OBJ files are **static meshes**, animations are achieved through:

1. **Procedural Animation** - Spring physics for organic movement
2. **Transform Animation** - Position/rotation changes over time
3. **Model Swapping** - Different OBJ files for different poses (if available)

The `Engine/Animation.hpp` provides:
- `Anim::Spring` - Single-value spring physics
- `Anim::Spring3D` - 3D spring physics
- `Anim::Tween<T>` - Smooth value interpolation
- `Anim::Ease::*` - Easing functions

---

## Texture Requirements

All models should have:
- Diffuse texture (main color)
- Optional: Normal map for extra detail
- Format: PNG or JPG

---

## Recommended Free Sources

1. **Sketchfab** (https://sketchfab.com) - Filter by "Downloadable" and "OBJ"
2. **Turbosquid** (https://turbosquid.com) - Has free section
3. **CGTrader** (https://cgtrader.com) - Free models available
4. **OpenGameArt** (https://opengameart.org) - Game-specific assets

---

## Missing Essential Models

Based on current assets, you still need:

### High Priority:
- [ ] Demon enemy model
- [ ] Boss demon model
- [ ] Sci-fi rifle with flashlight
- [ ] Lab reactor/core (Level 1 goal)
- [ ] Portal/Obelisk (Level 2 goal)

### Medium Priority:
- [ ] Floating platforms (parkour)
- [ ] Security doors (animated)
- [ ] Rooftop obstacles (AC units, pipes)

### Low Priority:
- [ ] Keycard model
- [ ] Additional corridor variations
- [ ] Decorative props

---

## File Structure

Place downloaded models in:
```
Doomers/assets/[model-name]/
    source/
        model.obj
        model.mtl
    textures/
        diffuse.png
        normal.png (optional)
```

The ResourceManager will automatically look for textures relative to the OBJ file.
