# CommonLibF4RD Example Plugin

This is a complete, buildable F4SE plugin project using
[CommonLibF4RD](https://github.com/Zzyxz/CommonLibF4RD).

Clone it when starting a new plugin, or compare it with an existing plugin when
moving to CommonLibF4RD. The same DLL can load on supported OG, NG, and AE
runtime families without an exact executable-version whitelist.

## Runtime names used in this guide

- **OG**: the original Fallout 4 `1.10.163` runtime family
- **NG**: the Fallout 4 `1.10.984` runtime family
- **AE**: the Fallout 4 `1.11.x` runtime family

CommonLibF4RD selects IDs and offsets for the active family. Mod authors do not
need separate OG, NG, and AE DLLs when their code and class-layout assumptions
support all three families.

## What this example contains

- a standalone CMake and vcpkg project;
- CommonLibF4RD included as a Git submodule;
- the F4SE plugin entry point and version export;
- metadata for address and structure independence;
- no hard-coded rejection of unknown patch numbers;
- simple examples for every `REL::ID` form;
- simple examples for `REL::VariantOffset`;
- a concrete `Actor::DoHitMe` automatic-callsite example;
- known offsets for calls that another plugin has already hooked (`or_offset`);
- optional hooks that disable one feature instead of stopping the game
  (`REL::try_resolve_callsite`);
- opt-in `.trace`, `.mapping`, and `.mapping.fail` diagnostics.

The example DLL only initializes F4SE and writes a log entry. The relocation
examples compile as reference code, but are not called and do not install hooks.

## Requirements

- Windows x64
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.21 or newer
- vcpkg
- F4SE matching the Fallout 4 runtime used for testing
- the CommonLibF4RD Runtime Database for in-game testing

Set `VCPKG_ROOT` to the directory containing your vcpkg installation.

## Clone and build

Clone recursively so the CommonLibF4RD submodule is included:

```text
git clone --recursive https://github.com/Zzyxz/CommonLibF4RD-ExamplePlugin.git
cd CommonLibF4RD-ExamplePlugin
```

If the repository was cloned without `--recursive`, initialize the submodule:

```text
git submodule update --init --recursive
```

Build a Release DLL:

```text
cmake --preset vs2022-windows-vcpkg
cmake --build --preset vs2022-release
```

The result is written to:

```text
build/vs2022/Release/F4RDExamplePlugin.dll
```

## Install and run

Install the DLL and Runtime Database like this:

```text
Data/
└─ F4SE/
   └─ Plugins/
      ├─ F4RDExamplePlugin.dll
      └─ f4rd-runtime.bin
```

Start Fallout 4 through F4SE. The normal plugin log is written under:

```text
Documents/My Games/Fallout4/F4SE/F4RDExamplePlugin.log
```

## Check your plugin with `.trace`

Create an empty file next to the DLL with the same base name:

```text
F4RDExamplePlugin.dll
F4RDExamplePlugin.trace
```

On the next launch, CommonLibF4RD overwrites the `.trace` file and records only
the IDs that this plugin actually requests. Each resolved ID includes its RVA.
A relocation using `VariantOffset` or `AUTO_CALLSITE` also records the selected
runtime slot, selected offset, final RVA, and whether the offset was fixed or
found automatically.

This is the normal diagnostic mode for testing a plugin on OG, NG, and AE.
Delete or rename the `.trace` file to disable it.

The untouched example does not execute its relocation examples, so it may have
no relocation entries. A real plugin produces entries when its features request
their IDs.

## Validate the complete database with `.mapping`

Create another empty marker file next to the DLL:

```text
F4RDExamplePlugin.dll
F4RDExamplePlugin.mapping
```

On the next launch, CommonLibF4RD overwrites it with a complete `ID RVA` mapping
for the currently running Fallout 4 executable. This is different from `.trace`:

| File | What it resolves |
| --- | --- |
| `F4RDExamplePlugin.trace` | Only IDs requested by this plugin |
| `F4RDExamplePlugin.mapping` | Every ID available for the current executable |

If complete mapping finds failures, CommonLibF4RD creates:

```text
F4RDExamplePlugin.mapping.fail
```

The failure file lists each unresolved ID and its reason. Full mapping can take
noticeable time, so use it only for dedicated development or validation runs.
Do not include `.trace`, `.mapping`, `.mapping.fail`, or PDB files in a normal
release package.

## One entry point for OG, NG, and AE

`src/main.cpp` uses the normal F4SE entry point:

```cpp
extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(
    const F4SE::LoadInterface* a_f4se)
{
    if (!Plugin::Initialize(a_f4se)) {
        return false;
    }

    return true;
}
```

OG, NG, and AE all enter the plugin through this function. There is no runtime
switch here. CommonLibF4RD selects the correct IDs and offsets when each
relocation is constructed.

## Why the plugin metadata matters

`src/Plugin.cpp` exports:

```cpp
data.addressIndependence =
    F4SE::PluginVersionData::kAddressIndependence_Signatures;

data.structureIndependence =
    F4SE::PluginVersionData::kStructureIndependence_1_10_980Layout |
    F4SE::PluginVersionData::kStructureIndependence_1_11_137Layout;
```

`addressIndependence` tells F4SE that the plugin uses relocations instead of
fixed executable addresses. A new patch number alone should therefore not make
F4SE reject the plugin.

`structureIndependence` is separate. It declares which C++ class-layout
families the plugin has been tested with. Runtime address resolution cannot make
an incompatible class layout or function ABI safe. Only advertise layouts that
the plugin really supports.

Initialization should fail when a required ID, callsite, interface, ABI, or
layout cannot be used safely, not simply because a patch number is unfamiliar.

## Understanding `REL::ID`

A Runtime Database ID is a stable name for a game function or object. It is not
an address and it is not a Fallout 4 version number. The resolver converts the
selected ID into the correct RVA for the executable currently running.

The examples below use `Actor::DoHitMe`:

- OG identifies `Actor::DoHitMe` with ID `881215`.
- NG and AE identify the same logical function with ID `2231148`.

### One ID: `REL::ID(AE)`

```cpp
constexpr REL::ID kActorDoHitMe{
    2231148
};
```

The runtime selection is:

| Runtime | What happens |
| --- | --- |
| OG | CommonLibF4RD tries the Runtime Database's verified automatic OG bridge for AE ID `2231148` |
| NG | Uses `2231148` directly |
| AE | Uses `2231148` directly |

This one-ID form can therefore work on all three families. On OG, however, it
depends on a verified bridge in the Runtime Database. If no safe OG mapping
exists, resolution fails with `og_bridge_failed`. CommonLibF4RD does not guess.

Use this compact form only after testing every runtime family supported by the
plugin. To guarantee an explicit OG choice, use the two-ID form.

### Two IDs: `REL::ID(OG, AE)`

```cpp
constexpr REL::ID kActorDoHitMe{
    881215,  // OG
    2231148  // NG and AE
};
```

Yes, this form also supports NG:

| Runtime | Selected ID |
| --- | --- |
| OG | `881215` |
| NG | `2231148` |
| AE | `2231148` |

The first value is always OG. The second value is shared by NG and AE. This is
the recommended form when OG needs a different ID but NG and AE use the same
one.

### Three IDs: `REL::ID(OG, NG, AE)`

```cpp
constexpr REL::ID kActorDoHitMe{
    881215,  // OG
    2231148, // NG
    2231148  // AE
};
```

Use this form when all three runtime families need an explicit value. NG and AE
may have different IDs; they happen to be identical for `Actor::DoHitMe`.

The order is always `OG, NG, AE`.

### Resolve the selected ID

```cpp
REL::Relocation<std::uintptr_t> actorDoHitMe{
    kActorDoHitMe
};

const auto address = actorDoHitMe.address();
```

The plugin supplies logical IDs. CommonLibF4RD selects the runtime value,
resolves it, validates it, and caches the result for later requests.

## Understanding `REL::VariantOffset`

An ID normally points to the beginning of a function. A hook may need an
instruction inside that function. `VariantOffset` selects the correct interior
offset for the active runtime family.

### One offset for every family

```cpp
REL::VariantOffset{ 0x8F7 }
```

OG, NG, and AE all use `0x8F7`.

### Separate OG and modern offsets

```cpp
REL::VariantOffset{
    0x921, // OG
    0x8F7  // NG and AE
}
```

The second value is shared by NG and AE.

### One explicit offset per family

```cpp
REL::VariantOffset{
    0x921, // OG
    0x930, // NG
    0x8F7  // AE
}
```

The order is again `OG, NG, AE`.

`VariantOffset` selects a runtime family, not a specific patch number. For
example, multiple AE patches use the AE slot. Use a fixed interior offset only
when it has been verified for the entire family. If the wanted instruction is a
call to another known function, automatic callsite discovery is usually more
resilient.

## Concrete `AUTO_CALLSITE` example: `Actor::DoHitMe`

Suppose a plugin wants to replace one particular call to `Actor::DoHitMe` but
must leave all other calls to `Actor::DoHitMe` untouched.

A callsite hook needs two different function IDs:

1. **The caller:** the surrounding function containing the call instruction to
   replace.
2. **The target:** `Actor::DoHitMe`, the function that instruction must call.

```cpp
// The function that contains the particular call we want to replace.
constexpr REL::ID kFunctionThatCallsActorDoHitMe{
    1546751, // surrounding caller function on OG
    2229323  // the same logical caller function on NG and AE
};

// The function that must be called at the hook location.
constexpr REL::ID kActorDoHitMe{
    881215, // Actor::DoHitMe on OG
    2231148 // Actor::DoHitMe on NG and AE
};
```

These ID pairs do not describe the same function. The first pair tells
CommonLibF4RD where to search. The second pair tells it what call to search for.

```cpp
REL::Relocation<std::uintptr_t> hookSite{
    kFunctionThatCallsActorDoHitMe,
    REL::VariantOffset{
        REL::AUTO_CALLSITE(kActorDoHitMe)
    }
};
```

At runtime, CommonLibF4RD:

1. resolves the surrounding caller function for OG, NG, or AE;
2. resolves `Actor::DoHitMe` for the same runtime;
3. searches only inside the surrounding caller;
4. finds a direct call whose destination is `Actor::DoHitMe`;
5. requires that match to be unique;
6. returns the address of that call instruction.

The plugin can replace that one call with its hook. It is not hooking the
`Actor::DoHitMe` function entry, and it is not changing every call to
`Actor::DoHitMe` in the game.

This is more update-resilient than `caller + fixedOffset` because inserted or
removed instructions may move the call while the caller-to-target relationship
remains intact.

### Multiple calls to the same target

The default `AUTO_CALLSITE` form deliberately fails if the caller contains more
than one matching call. If multiple calls are intentional and have been
verified, a specific occurrence can be selected:

```cpp
REL::AUTO_CALLSITE_FIRST(kActorDoHitMe);
REL::AUTO_CALLSITE_LAST(kActorDoHitMe);
REL::AUTO_CALLSITE_NTH(kActorDoHitMe, 2); // zero-based: the third match
```

`FIRST`, `LAST`, and `NTH` are less resilient than a unique match because an
update may insert or reorder calls. Always verify the selected call context and
the hook ABI on every supported layout family.

### Calls already hooked by another plugin: `or_offset`

The automatic search looks for a direct call whose destination is
`Actor::DoHitMe`. If another plugin has already replaced exactly that call with
its own hook, for example with `write_call`, the call now goes to that plugin's
code instead. The search then finds no match, or `FIRST`/`NTH` may silently pick
a different call.

`or_offset` adds the offset of the call for exact game versions on which it has
been checked:

```cpp
REL::Relocation<std::uintptr_t> hookSite{
    kFunctionThatCallsActorDoHitMe,
    REL::VariantOffset{
        // OG slot
        REL::AUTO_CALLSITE(kActorDoHitMe)
            .or_offset(0x921, REL::Version{ 1, 10, 163, 0 }),
        // NG slot
        REL::AUTO_CALLSITE(kActorDoHitMe)
            .or_offset(0x8F7, REL::Version{ 1, 10, 984, 0 }),
        // AE slot: only the checked AE versions
        REL::AUTO_CALLSITE(kActorDoHitMe)
            .or_offset(0x8F7, REL::Version{ 1, 11, 221, 0 }, REL::Version{ 1, 11, 240, 0 })
    }
};
```

On a listed version, CommonLibF4RD reads the instruction at that offset first and
uses it only if it is a call (or a jump, for a jump callsite) that

1. still goes to `Actor::DoHitMe`, or
2. goes to code outside `Fallout4.exe`, which means another plugin has already
   redirected this call.

In the second case, `write_call` returns the other plugin's hook as the original
function. When the new hook calls that original, both hooks run one after the
other and neither plugin breaks the other.

If the check fails, or the running version is not listed, the normal automatic
search runs exactly as without `or_offset`. A game update that moves the call is
therefore never patched at an outdated offset.

Guidelines:

- The offset is relative to the caller, like every other `VariantOffset` value.
- List only versions on which the offset was checked. Each `or_offset` accepts
  up to four versions.
- The plugin log shows an `F4RD NOTE` line when a known offset was used for a
  call that another plugin had already hooked, including that plugin's DLL
  name. No marker file is needed for this line.

### Optional hooks: `REL::try_resolve_callsite`

`REL::Relocation` stops the game with an error message when an ID or callsite
cannot be resolved. That is correct for anything the plugin cannot work without.

For an optional feature, `REL::try_resolve_callsite` returns an empty result
instead:

```cpp
const auto lookup = REL::try_resolve_callsite(
    kFunctionThatCallsActorDoHitMe,
    REL::VariantOffset{
        REL::AUTO_CALLSITE(kActorDoHitMe)
    });

if (!lookup) {
    logger::warn(
        "optional Actor::DoHitMe hook disabled: {} {}",
        REL::id_resolve_status_text(lookup.resolution.status),
        lookup.resolution.note);
    return;  // skip only this hook
}

const auto hookSite = *lookup.address;
```

`lookup.resolution.status` is the short reason, for example
`callsite_not_found`. `lookup.resolution.note` adds details when available, for
example why a known offset from `or_offset` was rejected. It accepts the same
`VariantOffset` values as `REL::Relocation`, including `AUTO_CALLSITE` with
`or_offset`.

Compilable versions of all relocation examples are in
`src/RelocationExamples.cpp`. They are deliberately not called by the example
plugin.

## Safe failure behavior

CommonLibF4RD fails instead of returning a guessed address when it cannot safely
resolve an ID or callsite. Useful failure reasons include:

- `og_bridge_failed`: a one-ID declaration has no verified OG bridge;
- `ng_bridge_failed`: an AE ID has no verified resolution for NG;
- `pattern_not_found`: no valid pattern matched the executable;
- `pattern_ambiguous`: more than one valid pattern match remained;
- `callsite_not_found`: the expected caller-to-target call no longer exists;
- `callsite_ambiguous`: more than one call matched when a unique call was
  required;
- `runtime_unavailable`: the known symbol does not exist on this runtime family.

Treat a required relocation failure as a reason to disable that feature or stop
plugin initialization safely. For optional hooks, `REL::try_resolve_callsite`
returns these reasons instead of stopping the game.

## Turn this example into your plugin

1. Rename `F4RDExamplePlugin` in `CMakeLists.txt`.
2. Update the project version and package metadata.
3. Rename the C++ namespace if desired.
4. Replace the demonstration IDs with IDs required by the plugin.
5. Resolve and validate required relocations before installing hooks.
6. Test OG, NG, and AE with a matching `.trace` marker.
7. Use `.mapping` only for dedicated full-database validation.
8. Package the DLL and required assets, but not PDB or diagnostic marker files.

See the full
[CommonLibF4RD feature guide](https://github.com/Zzyxz/CommonLibF4RD/blob/main/docs/FEATURES.md)
for resolver statuses, advanced callsite selectors, and migration details.

## License

This example is available under the MIT License. See [LICENSE](LICENSE).
