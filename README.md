# 🔥 LOVEMACHINE CS:S v92 (Steam & Non-Steam)

> **Next-Generation Internal Framework & Visuals Engine for Counter-Strike: Source**  
> Complete redesign featuring modern UI architectures, advanced combat algorithms, 3D character customizations, and robust server bypasses.

---

## ✨ Key Features & Enhancements

### 🎯 1. Combat & Ragebot
* **Magic Bullet & Instant Hitscan**: Ultra-precise target acquisition targeting head, neck, chest, and pelvis with multi-point scanning.
* **100% Pinpoint Accuracy While Moving**: Active `Autostop` counter-strafing combined with standalone recoil & spread compensation (`NoRecoil` & `NoSpread`).
* **Anti-Aim Engine**:
  * Spinbot (Silent server-side desync or visual client spin).
  * Pitch: Down (Emotion 89°), Up (-89°), Zero.
  * Yaw: Backward (180°), Jitter, Sideways (90°).
* **Autowall (Bullet Penetration)**: Accurate wall-penetration calculation with surface material analysis and minimum damage thresholds.

### 🏹 2. Legitbot & Backtrack Engine
* **Humanized Smooth Aimbot**: Curve-based smoothing with recoil control system (RCS) pitch & yaw multipliers.
* **14-Tick Backtrack**: Extended historical position matrix with lag compensation and customizable tick records.
* **Intelligent Auto Knife (Knifebot)**: Automatic range detection (65-75 units) with health/armor analysis for automatic right-click backstab kills.

### 👁️ 3. Visuals & ESP Engine
* **2D & 3D ESP Overlays**: Bounding boxes, skeleton bones, dynamic health/armor/ammo bars, active weapons, and snaplines.
* **Tactical Player Flags**: Real-time status indicators on bounding boxes (`HK` armor, `KIT` defuser, `BLIND` flashed, `DUCK`, and `$MONEY`).
* **Out of FOV (OOF) Arrows**: Rotating offscreen pointer triangles around screen center pointing towards flanking/behind enemies with distance indicators.
* **Footstep Sound ESP**: 3D expanding ripple rings and `[STEP]` text markers rendered at footstep & gunshot origins.
* **Rainbow Trail**: Animated HSV spectrum light trail flowing behind the player during movement with speed controls.
* **3D Asian Rice Hat**: Customizable 3D conical hat rendering above player heads with radius and height sliders.
* **Thirdperson Without `sv_cheats`**: Client-side view setup override with ray-traced collision against walls (works on any server).
* **Spectator Mode ESP**: ESP remains 100% active and visible when dead or spectating other players.

### 🎭 4. Custom 3D Character Models
* **Plug-and-Play `scripts/models/` System**: Automatically installs custom models into the game folder on injection.
* **Supported Models**:
  * `Phoenix Terrorist (T)`
  * `Leet Krew (T)`
  * `SAS Gasmask (CT)`
  * `GIGN SWAT (CT)`
  * `Hostage (Scientist)`
  * `Cissia ZZZ (Zenless Zone Zero)`
* **Local Player Only Option**: Apply transformations exclusively to your character in thirdperson while keeping enemies standard.

### 🏃 5. Movement & Server Bypasses
* **Fake Duck**: 14-tick packet choke cycle allowing standing view shooting while maintaining crouching hitbox.
* **Slowwalk**: Velocity limiting (35 units/s) for silent movement with zero running spread penalty.
* **Bunnyhop & Auto-Strafer**: Perfect jump timing and air-strafe acceleration.
* **Fake Ping / Network Latency Spike**: NetChannel datagram sequence manipulation for high ping simulation and expanded backtrack windows.
* **SV_Pure & Anti-SMAC Bypasses**: Pitch clamping (-89° to +89°), clean angle normalization, and pure check circumvention.

### 🎨 6. Multi-Architecture UI Menu (Default Key: `INSERT`)
* **Gamesense (Skeet Classic)**: Emerald accent, top rainbow glow bar, and iconic icon navigation.
* **Neverlose 2.0 HUD**: Electric cyan theme, categorized subtabs, profile manager, and interactive presets.
* **Aternos 3D Visualizer**: Sleek dark layout with live 3D ESP mannequin preview window.
* **Synthetic Honeycomb**: Futuristic space galaxy theme with interactive 7-hexagon navigation.

---

## 📂 Project Structure

```
lovemachine_css/
├── lovemachine/          # Core C++ cheat source files (Hooks, Visuals, Ragebot, Legitbot)
├── scripts/              # External customizable assets
│   ├── configs/          # Config profiles (.ini)
│   └── models/           # Custom 3D character packages (Cissia ZZZ, etc.)
├── assets/               # Built-in fonts and textures
└── lovemachine_test      # Standalone UI preview executable (macOS / Linux / Windows)
```

---

## 🔨 Building & Injecting

### Windows (Visual Studio)
1. Open `lovemachine.sln` in **Visual Studio 2019 / 2022**.
2. Select **Release | x86** configuration.
3. Build the solution (`lovemachine.dll`).
4. Run CS:S (`hl2.exe`) and inject using the diagnostic injector or any standard manual map injector.

### macOS / Linux Standalone UI Preview Test
```bash
clang++ -std=c++17 -O2 lovemachine/main_mac.cpp lovemachine/menu.cpp \
    lovemachine/imgui/imgui.cpp lovemachine/imgui/imgui_draw.cpp \
    lovemachine/imgui/imgui_tables.cpp lovemachine/imgui/imgui_widgets.cpp \
    lovemachine/imgui/backends/imgui_impl_sdl2.cpp \
    lovemachine/imgui/backends/imgui_impl_opengl3.cpp \
    -iquote lovemachine -Ilovemachine/imgui -Ilovemachine/imgui/backends \
    $(sdl2-config --cflags --libs) -framework OpenGL -o lovemachine_test
./lovemachine_test
```

---

## 📜 Credits & Acknowledgments
* **Original Project**: Lovemachine CS:S
* **Developers & Contributors**: MinhYz, Xarex, Kolo, Wav, and the UnknownCheats community.
