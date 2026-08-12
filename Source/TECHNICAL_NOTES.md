# Technical notes

Target: **SOVIET64.exe v1.1.1.9**  
TesmioLoader API: **4**

## Native construction seam

Free Heliports intercepts `FUN_140159070` at RVA `0x159070`, WRSR's normal construction updater:

```text
void FUN_140159070(game, building, float progressPulse, char mode)
```

WRSR's own complete-all-construction path sets a building's progress field at `building + 0x604` to `1.0`, then calls this function with a `0.1` progress pulse. Free Heliports uses the same transition for newly placed `FreeHeliportParking` instances.

## Scope checks

The completion override is applied only when all of the following are true:

- the building is unfinished and not already inside its completion transition;
- its type descriptor reports `TYPE_AIRPLANE_PARKING` (`0x2F`);
- its descriptor name is exactly `FreeHeliportParking`, allowing only a Workshop path prefix;
- both the static type descriptor and live building instance contain valid non-empty construction-stage vectors.

All other calls are forwarded without modification.

## Hook compatibility

For an untouched target, the plugin validates the exact 16-byte WRSR 1.1.1.9 prologue and passes those instruction-aligned bytes to TesmioLoader's guarded inline-hook installer. The hook fails closed on any prologue mismatch.

If the target already contains TesmioLoader's standard `FF 25 [rip+0]` absolute-indirect stub, the plugin validates its callable target and chains it. Unknown target layouts are not overwritten; initialization fails closed and records the failure in `tesmioloader.log`.

HDO hooks vehicle compatibility, Distribution Office task handling, menus, residence and UI paths. Free Heliports does not use those locations and has no HDO dependency.

## Asset contract

`FreeHeliportParking/building.ini` must retain one correctly formed zero-cost stage:

```text
$COST_WORK SOVIET_CONSTRUCTION_GROUNDWORKS 0.0
```

Do not leave orphaned `$COST_WORK_BUILDING*`, `$COST_WORK_VEHICLE_STATION*` or `$COST_RESOURCE_AUTO` directives without a parent `$COST_WORK` stage.
