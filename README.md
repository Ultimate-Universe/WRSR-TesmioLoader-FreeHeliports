# Free Heliports

Free Heliports is a TesmioLoader plugin and Workshop building pack for **Workers & Resources: Soviet Republic**. It adds compact grass-and-dirt helicopter parking and cargo heliports that cost nothing and are placed instantly.

Current release: **1.1.0**  
Supported game version: **WRSR 1.1.1.9 (64-bit Windows)**  
TesmioLoader interface: **API 4**

## Included buildings

### Free Heliport Parking

- Stores one light or heavy helicopter.
- Costs nothing and completes immediately when the plugin is active.
- Requires no labour, construction materials or utilities.

### Free Cargo Heliport

- Handles the standard cargo-heliport cargo categories.
- Includes fuel storage, road access and three factory/storehouse connections.
- Costs nothing and is placed instantly.
- Loads and unloads at 10% of the normal cargo-heliport speed.

## Why the plugin is required

WRSR requires helicopter parking to retain construction-stage metadata before helicopters weighing 10 tonnes or more are accepted. A genuine zero-work stage does not advance by itself.

`free_heliports.dll` recognises only `FreeHeliportParking`, completes its valid zero-cost stage, and forwards the building through WRSR's native construction-completion path. It does not modify helicopter weight, vehicle purchasing, ownership, residence assignment or cargo behaviour.

## Requirements

- Workers & Resources: Soviet Republic **1.1.1.9** on 64-bit Windows.
- [TesmioLoader](https://steamcommunity.com/sharedfiles/filedetails/?id=3773169177) with plugin API 4.
- Steam Workshop item [`3779842468`](https://steamcommunity.com/sharedfiles/filedetails/?id=3779842468).

No DLC is required. Helicopter Distribution Office (HDO) is not required.

## Installation

1. Subscribe to the Free Heliports Workshop item and TesmioLoader.
2. Copy:

   ```text
   Steam\steamapps\workshop\content\784150\3779842468\plugins\free_heliports.dll
   ```

   to:

   ```text
   Steam\steamapps\common\SovietRepublic\tesmioloader\build\plugins\
   ```

3. Launch the game through `tesmiolauncher.exe` and enable `free_heliports.dll`.

If the plugin is missing or disabled, a newly placed `FreeHeliportParking` will remain at its zero-cost construction stage. Existing completed heliports are not removed or converted.

## Compatibility and troubleshooting

- Version 1.1.0 is specifically built for WRSR 1.1.1.9 and TesmioLoader API 4. The plugin refuses to install its hook if the game code does not match the audited target.
- The plugin changes only the construction completion of `FreeHeliportParking`; it does not run a per-frame building scan.
- HDO is optional. Experimental HDO builds for WRSR 1.1.1.9 may interfere with helicopter purchase or residence assignment. If a purchase takes payment but no helicopter appears, disable or update HDO; that path is outside Free Heliports.
- If another compatible TesmioLoader plugin already owns the same construction seam, Free Heliports can chain its standard hook. Unknown modifications are left untouched.

For a bug report, include reproduction steps, the game and TesmioLoader versions, the list of enabled plugins, and `tesmioloader.log`.

## Repository layout

- `ModFiles/` contains the complete Workshop/runtime payload.
- `Source/` contains the plugin source and only the scripts needed to build and verify the DLL.
- `WorkshopDescription.txt` contains the Steam Workshop description.
- `STEAM_WORKSHOP_CHANGELOG.txt` contains a ready-to-paste Workshop update note.

Build instructions are in [`Source/BUILD.md`](Source/BUILD.md). Audited hook and structure details are in [`Source/TECHNICAL_NOTES.md`](Source/TECHNICAL_NOTES.md).

## License

The plugin source code is distributed under the GNU General Public License version 3; see [`LICENSE`](LICENSE).

The building models, textures and other game-derived assets remain subject to the rights of their original rights holders and are not relicensed by the GPL.

This project is not affiliated with or endorsed by 3DIVISION, Hooded Horse, Valve or the TesmioLoader author.
