# Technical notes

Target: **SOVIET64.exe v1.1.1.7**  
Reference executable SHA-256: `ec05bb6257da31cfcaec639c8462683ee7bcf26158e0751a29a2d48025169522`  
TesmioLoader API: **3**

## Native construction seam

Free Heliports intercepts `FUN_140159000` at RVA `0x159000`, WRSR's normal construction updater:

```text
void FUN_140159000(game, building, float progressPulse, char mode)
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

For an untouched target, the plugin decodes only ordinary position-independent MSVC prologue instructions and passes the captured live bytes to TesmioLoader's normal guarded inline-hook installer.

If the target already contains TesmioLoader's standard `FF 25 [rip+0]` absolute-indirect stub, the plugin validates its callable target and chains it. Unknown target layouts are not overwritten; initialization fails closed and records the failure in `tesmioloader.log`.

HDO hooks vehicle compatibility, Distribution Office task handling, menus, residence and UI paths. Free Heliports does not use those locations and has no HDO dependency.

## Asset contract

`FreeHeliportParking/building.ini` must retain one correctly formed zero-cost stage:

```text
$COST_WORK SOVIET_CONSTRUCTION_GROUNDWORKS 0.0
```

Do not leave orphaned `$COST_WORK_BUILDING*`, `$COST_WORK_VEHICLE_STATION*` or `$COST_RESOURCE_AUTO` directives without a parent `$COST_WORK` stage.
