# Free Heliports

A TesmioLoader plugin and Workshop building pack for **Workers & Resources: Soviet Republic** that adds compact grass-and-dirt helicopter parking and cargo heliports which are free and placed instantly.

Current version: **1.0.0**

## Included buildings

### Free Heliport Parking

- Stores one helicopter.
- Supports light and heavy helicopters.
- Free and placed instantly.
- Requires no labour, construction materials or utilities.

### Free Cargo Heliport

- Handles the standard cargo-heliport cargo categories.
- Includes fuel storage.
- Retains road access and three factory/storehouse connections.
- Free and placed instantly.
- Loads and unloads at 10% of the normal cargo-heliport speed.

## Why the plugin is required

WRSR requires a helicopter parking building to retain construction-stage metadata before helicopters weighing 10 tonnes or more are considered valid for it. A genuinely zero-work stage does not complete by itself.

`free_heliports.dll` recognises only `FreeHeliportParking`, marks its zero-cost stage complete when the building is placed, and forwards the building through WRSR's normal construction-completion routine. It does not change helicopter weight, vehicle compatibility, ownership or cargo behaviour.

## Requirements

- Workers & Resources: Soviet Republic v1.1.1.7 on 64-bit Windows.
- [TesmioLoader](https://steamcommunity.com/sharedfiles/filedetails/?id=3773169177), plugin API 3.
- Steam Workshop item `3779842468`.

No DLC is required.

## Installation

Subscribe to the Workshop item so the building assets are installed normally.

Then copy:

```text
Steam\steamapps\workshop\content\784150\3779842468\plugins\free_heliports.dll
```

to:

```text
Steam\steamapps\common\SovietRepublic\tesmioloader\build\plugins\
```

Launch the game through `tesmiolauncher.exe` and make sure `free_heliports.dll` is enabled.

## Compatibility

- Helicopter Distribution Office is not required.
- Free Heliports uses a separate construction-lifecycle hook and can run with HDO enabled or disabled.
- If another plugin has already installed TesmioLoader's standard inline-hook stub at the same construction updater, Free Heliports chains the existing callable.
- Unknown or unsafe changes at the hook target are left untouched and cause the plugin to decline activation.
- The plugin does not run a per-frame scan and does not modify other buildings.

If the plugin is missing or disabled, newly placed `FreeHeliportParking` assets will remain at their zero-cost construction stage. Existing completed heliports are not converted or removed.

## Source and building

The player-facing Workshop/runtime files are in `ModFiles/`. Complete plugin source, build scripts and reverse-engineered notes are in `Source/`.

Build instructions are in [`Source/BUILD.md`](Source/BUILD.md). The native hook and structure notes are in [`Source/TECHNICAL_NOTES.md`](Source/TECHNICAL_NOTES.md).

Steam Workshop:

https://steamcommunity.com/sharedfiles/filedetails/?id=3779842468

## Bug reports

Report problems through the GitHub issue tracker. Include:

- a concise description and reproduction steps;
- whether the problem affects parking, cargo handling or plugin loading;
- whether HDO or other TesmioLoader plugins are enabled;
- `tesmioloader.log` from the affected launch;
- the game and TesmioLoader versions.

## License

The plugin source code is distributed under the GNU General Public License version 3. See [`LICENSE`](LICENSE).

The building models, textures and other game-derived asset files remain subject to the rights of their original rights holders and are not relicensed by the GPL.

This project is not affiliated with or endorsed by 3DIVISION, Hooded Horse, Valve or the TesmioLoader author.
