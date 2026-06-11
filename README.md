# Prog4Pengo

Prog4Pengo is my project for the exam assignment in the course Programming 4 at DAE. It is a recreation of the 80's arcade game Pengo, using a self made game engine based on [Minigin](https://github.com/avadae/minigin).

**Source control:** https://github.com/YriDeLange/Prog4Pengo

[![Build Status](https://github.com/YriDeLange/Prog4Pengo/actions/workflows/cmake.yml/badge.svg)](https://github.com/YriDeLange/Prog4Pengo/actions/workflows/cmake.yml)
[![Build Status](https://github.com/YriDeLange/Prog4Pengo/actions/workflows/emscripten.yml/badge.svg)](https://github.com/YriDeLange/Prog4Pengo/actions/workflows/emscripten.yml)
[![GitHub Release](https://img.shields.io/github/v/release/YriDeLange/Prog4Pengo?logo=github&sort=semver)](https://github.com/YriDeLange/Prog4Pengo/releases/latest)

There is an Emscripten build available, which can be found [here](https://yridelange.github.io/Prog4Pengo/).

## Engine design choices

The solution is split into two projects: **Minigin** (static library, the engine) and
**Game** (executable, everything Pengo-specific). The guiding rule for what goes where
is *mechanism vs. policy*: the engine provides mechanisms (components, input binding,
scenes, sound, application states) and knows nothing about Pengo; the game supplies the
policy (what a Sno-Bee is, what a level file means, what the scoring rules are).

### Component model & game objects
`GameObject` owns a vector of `Component`s (Update/FixedUpdate/Render/RenderUI) and
supports parent/child hierarchies with lazy world-position evaluation (dirty flag
propagated to children). Everything visible or behavioural in the game — Pengo, Sno-Bees,
ice blocks, HUD widgets — is a GameObject composed from components.

### Game loop & timing
`Minigin::RunOneFrame` runs the classic loop: input → application state machine →
fixed update (fixed 60 Hz timestep with lag accumulation, for deterministic logic) →
variable update (frame delta) → render. `GameTime` owns the clock.

### Application state machine (State pattern, engine level)
`GameState`/`GameStateMachine` model the application flow (Menu → Playing → GameOver).
States own their scene and their input bindings; a state requests its own replacement by
*returning* the next state from `Update`, and the machine applies the swap between
frames so a state never destroys itself mid-call. The same pattern appears game-side at
entity level: Pengo (Standing/Walking/Pushing/Dying) and the Sno-Bee
(Spawning/Wandering/Eating/Stunned/Carried/Crushed) are both state machines.

### Input (Command pattern)
`InputManager` binds keyboard scancodes and gamepad buttons (per controller index, four
supported) to `Command` objects, on three trigger types (Down/Up/Pressed). Game states
follow a strict *bind on OnEnter, unbind on OnExit* discipline and track exactly what
they bound, so no command can outlive the GameObject it points at. `FunctionCommand`
wraps a lambda for one-off bindings (menu navigation, F-keys, Versus possession input).
The gamepad has two implementations behind a pImpl: XInput on Windows and SDL_Gamepad
for the web build, selected at compile time.

### Events (Observer pattern)
`Subject` implements observers as id-keyed `std::function` handlers. `HealthComponent`
and `PointsComponent` notify hashed event ids (compile-time sdbm hash); the HUD
(lives/score displays) and game systems subscribe. This keeps gameplay components
decoupled from presentation.

### Sound (Service Locator + threading)
`ServiceLocator` hands out a `SoundSystem` interface with a `NullSoundSystem` fallback,
so gameplay code never checks for null audio. The real implementation
(`SoundSystemImpl`, pImpl) runs SDL3_mixer and satisfies the threading requirement: on
desktop, `Play()` only pushes a request onto a mutex-guarded queue and a dedicated
worker thread (condition variable driven) performs the actual mixing calls, so audio
never stalls the game loop. Music runs on a dedicated looping track with pause/resume
(it ducks during Pengo's death jingle), and F2 toggles a master-gain mute.

### Resources & levels
`ResourceManager` caches textures and fonts by key (fonts per size).
Levels are plain text files in `Data/` — a 13×15 character grid, one character per
cell: `B` ice block, `D` diamond, `E` egg block, `P` Pengo spawn, `S` Sno-Bee spawn,
`.` empty. `LevelLoader` parses the file, builds the blocks and returns spawn data;
the active `PlayingState` decides what to do with it based on the game mode. Adding a
level is adding a file: the Sno-Bee budget, diamonds and spawns all derive from its
contents.

## Game architecture notes

- **LevelGrid** (game-side singleton) is the single source of truth for cell occupancy;
  blocks register/unregister themselves and movement queries go through it.
- **Registries** (`SnoBeeRegistry`, `PlayerRegistry`) let systems iterate live entities
  without owning them; objects self-register in their constructor and unregister in
  their destructor, so a raw pointer found in a registry is always valid *this frame*.
- **Versus possession:** Player 2 commands the swarm rather than one body. Input routes
  through a possession pointer; when the controlled Sno-Bee dies, control hops to the
  nearest living one (or waits for the next egg to hatch and takes that). The pointer is
  validated against the registry by address before every dereference, which closes the
  one-frame window between an entity dying and the possession logic noticing.
- **Scoring** follows the original Sega manual: crush 400 / double 1600 / triple 3200,
  egg block 500, stunned catch 100, diamond line 10,000 (5,000 along a wall, plus a
  global stun), and the sub-60-second time bonus table.
- **High scores** are a persistent top 10 with 3-letter arcade name entry (letter
  cycling, fully playable on keyboard and gamepad), stored in `highscores.txt` next to
  the executable.

## Controls

| Action | Keyboard | Gamepad |
|---|---|---|
| Move | WASD / arrow keys | D-pad |
| Push / break block | Space | X |
| Menu navigate / confirm | WASD+arrows / Enter, Space | D-pad / A |
| Skip level | F1 | — |
| Mute / unmute | F2 | — |

Modes: **Single player** (keyboard or gamepad), **Co-op** (two Pengos: keyboard +
gamepad, or two gamepads), **Versus** (Pengo vs. a player-possessed Sno-Bee:
keyboard/gamepad 1 vs. gamepad 0).

## Building

CMake ≥ 3.26 with the provided presets (Visual Studio 2022+, Ninja). Dependencies
(SDL3, SDL3_ttf, SDL3_mixer, glm, ImGui/ImPlot) are fetched automatically via
FetchContent. All targets build at warning level 4 with warnings as errors.
The `Data/` folder is copied next to the executable post-build.