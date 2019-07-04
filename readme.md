# aoc-farm-auto-reseed

A little mod that adds the farm auto-reseeding feature from AoE2: Definitive Edition to UserPatch 1.5.

This mod works together with UserPatch 1.5 and the [aoc-mmmod][] loader.

## Usage

Install the [aoc-mmmod][] loader in your mod's `Data\` folder. Grab `aoc-farm-auto-reseed.dll` from the [releases](https://github.com/SiegeEngineers/aoc-farm-auto-reseed/releases) page, and place it in your mod's `mmmods\` folder.

```bash
Games\Your Mod Name\Data\language_x1_p1.dll # the mmmod loader
Games\Your Mod Name\Data\language_x1_m.dll # optionally, language file with custom strings
                                           # (with aoc-language-ini, this is not needed)
Games\Your Mod Name\mmmods\aoc-farm-auto-reseed.dll # the farm reseeding module!
```

> **Note** This mod is sync-breaking. If you watch a game that was recorded _with_ farm-auto-reseed in a version of UserPatch _without_ it, you will get out of sync errors.

## Build

This project can only be built with MinGW GCC compilers at this time.

To create a debug build (with some logging):

```
cmake -S . -B build
cmake --build build
```

To create a release build (smaller and faster and no logging):

```
cmake -S . -B build -DCMAKE_RELEASE_TYPE=Release
cmake --build build
```

## License

[LGPL-3.0](./LICENSE.md)

[aoc-mmmod]: https://github.com/SiegeEngineers/aoc-mmmod
