# ⚡ Cyberpunk Neon Block Puzzle

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++17" />
  <img src="https://img.shields.io/badge/SFML-2.6.2-8CC445?style=for-the-badge&logo=sfml&logoColor=white" alt="SFML" />
  <img src="https://img.shields.io/badge/CMake-3.20%2B-064F8C?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake" />
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-blue?style=for-the-badge" alt="Platform" />
  <img src="https://img.shields.io/badge/Audio-Procedural%20Synthesis-blueviolet?style=for-the-badge" alt="Audio" />
  <img src="https://img.shields.io/badge/License-MIT-success?style=for-the-badge" alt="License" />
</p>

---

## 🎮 Overview

A high-performance, responsive falling-block arcade game engineered from scratch in **modern C++17** using **SFML 2.6.2**. 

Designed with an electric **Cyberpunk / Synthwave neon aesthetic**, the engine combines guideline-compliant competitive mechanics (Super Rotation System, 7-bag randomizer, DAS/ARR responsive inputs) with modern visual polish, dynamic particle bursts, haptic screen shake, and a **100% procedural in-memory sound synthesizer** requiring zero external audio assets.

---

## ✨ Key Highlights

### 🕹️ Guideline-Compliant Competitive Engine
- **Super Rotation System (SRS):** Complete official wall-kick and floor-kick matrices for all tetromino shapes ($I, J, L, O, S, T, Z$), enabling wall kicks and tight spins.
- **7-Bag Randomizer:** Guarantees fair piece sequences and eliminates piece starvation droughts.
- **Lock Delay (0.5s):** 500ms sliding window with up to 15 move/rotation resets before piece locking.
- **Fine-Tuned DAS & ARR:** 160ms Delayed Auto Shift with rapid 33ms Auto Repeat Rate (~30 Hz) for snappy maneuvering.
- **Arcade Scoring Model:**
  - Single (1 Line): `100 × Level`
  - Double (2 Lines): `300 × Level`
  - Triple (3 Lines): `500 × Level`
  - Quad Clear: `800 × Level` (with screen flash & celebratory banner)
  - Soft Drop: `+1 pt / row` | Hard Drop: `+2 pts / row`

### 🌌 Cyberpunk Neon Visuals & Particle Engine
- **Glossy 3D Neon Bevels:** Multi-layer block rendering featuring illuminated specular borders, inner face glow, and arcade bevel accents.
- **Projected Ghost Matrix:** Translucent real-time landing silhouette with high-contrast neon outlines.
- **Dynamic Particle Physics:** Velocity-driven neon spark bursts exploding along cleared rows.
- **Impact Screen Shake:** Traumatic camera oscillations on massive line clears and hard drops.
- **High-Tech HUD:** 3-piece Next Queue, Hold Chamber, dynamic rolling Score counter, Level scaling, and persistent High Score tracking (`highscore.txt`).

### 🔊 100% Procedural In-Memory Audio Synthesizer
- **Zero Asset Dependencies:** No external `.wav` or `.ogg` files required. Every sound wave is mathematically synthesized at runtime using custom sample generators:
  - *Move & Shift:* Crisp modulated micro-chirps
  - *SRS Rotation:* Pitch-swept resonant blips
  - *Hard Drop:* Low-frequency punchy sub-bass transient
  - *Line Clear:* Harmonic chord arpeggios (C5 – E5 – G5)
  - *Quad Clear:* Triumphant fanfare with rich vibrato and octave harmonics
  - *Game Over:* Descending retro frequency sweep
- **16-Voice Polyphony:** Audio pooling prevents sound cutoff during rapid drops and simultaneous clears.

---

## 🎨 Neon Color Palette

| Piece | Shape | Accent Color | Hex Code |
| :---: | :---: | :--- | :--- |
| **I** | `[][][][]` | Electric Cyan | `#00F0FF` |
| **O** | `[][] / [][]` | Bright Gold | `#FFD700` |
| **T** | `[][][] / []` | Neon Violet | `#AF3CFF` |
| **S** | `[][] / [][]` | Acid Green | `#32F050` |
| **Z** | `[][] / [][]` | Crimson Red | `#FF2D4B` |
| **J** | `[] / [][][]` | Cobalt Blue | `#1E64FF` |
| **L** | `[][][] / []` | Solar Orange | `#FF8214` |

---

## ⌨️ Controls

| Key | Function |
| :--- | :--- |
| <kbd>←</kbd> / <kbd>→</kbd> | Move Left / Right (with DAS/ARR auto-shift) |
| <kbd>↓</kbd> | Soft Drop (`+1` point/cell) |
| <kbd>Space</kbd> | Hard Drop (Instant drop & lock, `+2` points/cell) |
| <kbd>↑</kbd> / <kbd>X</kbd> | Rotate Clockwise (SRS Wall Kicks) |
| <kbd>Z</kbd> / <kbd>Ctrl</kbd> | Rotate Counter-Clockwise (SRS Wall Kicks) |
| <kbd>C</kbd> / <kbd>Shift</kbd> | Hold Piece (swap current piece with hold chamber) |
| <kbd>P</kbd> | Pause / Unpause |
| <kbd>R</kbd> | Instant Restart |
| <kbd>M</kbd> | Toggle Audio Mute |

---

## 🏗️ Project Architecture

```
Tetris/
├── CMakeLists.txt         # Automated CMake setup with SFML FetchContent
├── .gitignore             # Comprehensive C++/CMake/IDE ignore rules
├── README.md              # Project documentation and guide
├── include/
│   ├── Config.hpp         # Board metrics, palette constants, gameplay timings
│   ├── Tetromino.hpp      # Piece geometries, SRS kick tables, block rendering
│   ├── Board.hpp          # Matrix state, collision grid, line clear anims
│   ├── PieceBag.hpp       # 7-bag randomizer distribution queue
│   ├── Particles.hpp      # Spark physics engine and camera shake controller
│   ├── SoundManager.hpp   # Procedural waveform generator & polyphonic voice pool
│   └── Game.hpp           # Game loop, state coordinator, and HUD interface
└── src/
    ├── main.cpp           # Entry point
    ├── Game.cpp           # Loop logic, event dispatch, and UI rendering
    ├── Board.cpp          # Grid collision and line resolution
    ├── Tetromino.cpp      # Rotation matrix and kick implementation
    ├── PieceBag.cpp       # Bag generator
    ├── Particles.cpp      # Particle math and screen-shake integration
    └── SoundManager.cpp   # DSP synthesis math and buffer allocation
```

---

## 🚀 Building & Running

### Prerequisites
- **C++17** compatible compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- **CMake 3.20** or higher
- Internet connection on first build (CMake `FetchContent` downloads SFML 2.6.2 automatically)

### 📦 Build Instructions

#### Windows (MinGW / GCC)
```powershell
# Configure build with MinGW
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Compile all targets
cmake --build build -j

# Launch the game
.\build\Tetris.exe
```

#### Windows (Visual Studio / MSVC)
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\Release\Tetris.exe
```

#### Linux (Debian / Ubuntu / Arch)
```bash
# Install audio & graphical dependencies for SFML
sudo apt-get install libxrandr-dev libxcursor-dev libudev-dev libopenal-dev libflac-dev libvorbis-dev

# Build & Run
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/Tetris
```

---

## ⚙️ Customization & Tuning

Want to customize the drop speed, colors, or timings? Open [`include/Config.hpp`](include/Config.hpp):

- `LOCK_DELAY`: Window before a piece locks down (default `0.5f`).
- `DAS_DELAY` & `ARR_INTERVAL`: Adjust horizontal responsiveness.
- `BLOCK_SIZE`: Scale matrix rendering size.
- `COLOR_*`: Swap out hex RGB colors for custom skins.

---

## 📜 License

Distributed under the **MIT License**. See project files for more information.
