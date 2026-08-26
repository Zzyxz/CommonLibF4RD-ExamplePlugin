# CommonLibF4RD Example Plugin

This repository is a complete, minimal F4SE plugin project built with
[CommonLibF4RD](https://github.com/Zzyxz/CommonLibF4RD).

It is intended to be cloned, built, and used as the starting point for a real
plugin. The generated DLL loads on supported OG, NG, and AE runtime families
without an exact executable-version whitelist.

## What this example demonstrates

- a standalone CMake and vcpkg project;
- CommonLibF4RD included as a Git submodule;
- a valid `F4SEPlugin_Version` export;
- address- and structure-independence metadata;
- no hard-coded runtime-version rejection;
- the one-, two-, and three-ID forms of `REL::ID`;
- `REL::VariantOffset`;
- automatic hook callsite discovery;
- `.trace`, `.mapping`, and `.mapping.fail` diagnostics.

The example plugin only initializes F4SE and writes a log entry. The relocation
examples are compiled but are not executed and do not install hooks.

## Requirements

- Windows x64
- Visual Studio 2022 with Desktop development with C++
- CMake 3.21 or newer
- vcpkg
- F4SE matching the game runtime used for testing
- the CommonLibF4RD Runtime Database for in-game testing

Set `VCPKG_ROOT` to the directory containing your vcpkg installation.

## Clone

Clone recursively so the CommonLibF4RD submodule is included:

```text
git clone --recursive https://github.com/Zzyxz/CommonLibF4RD-ExamplePlugin.git
cd CommonLibF4RD-ExamplePlugin
```

If the repository was cloned without `--recursive`, initialize the submodule:

```text
git submodule update --init --recursive
```

## Build

```text
cmake --preset vs2022-windows-vcpkg
cmake --build --preset vs2022-release
```

The Release DLL is written to:

```text
build/vs2022/Release/F4RDExamplePlugin.dll
```

Install it under:

```text
Data/F4SE/Plugins/F4RDExamplePlugin.dll
```

The Runtime Database is distributed separately and must be installed as:

```text
Data/F4SE/Plugins/f4rd-runtime.bin
```

## Why the plugin metadata matters

The example exports this metadata in `src/Plugin.cpp`:

```cpp
data.addressIndependence =
    F4SE::PluginVersionData::kAddressIndependence_Signatures;

data.structureIndependence =
    F4SE::PluginVersionData::kStructureIndependence_1_10_980Layout |
    F4SE::PluginVersionData::kStructureIndependence_1_11_137Layout;
```

`addressIndependence` tells the F4SE loader that the plugin does not depend on
one fixed executable address table. CommonLibF4RD resolves requested IDs for the
running executable.

`structureIndependence` declares which game structure-layout families the
plugin supports. Only advertise layouts that the plugin has actually been
tested with. Address resolution cannot make incompatible C++ class layouts
safe.

Do not add an exact runtime-version check just because a patch number is new.
Initialization should fail only when a required ID, callsite, interface, or
layout cannot be used safely.

## Runtime-aware IDs

The numbers passed to `REL::ID` are Runtime Database IDs. They are not Fallout
version numbers. Each number identifies the same logical function or object for
a particular game generation.

| Form | Meaning |
| --- | --- |
| `REL::ID(AE)` | One AE ID. NG uses the same ID. OG is resolved automatically only when a verified OG mapping exists. |
| `REL::ID(OG, AE)` | The first ID is for OG. The second ID is used by both NG and AE. |
| `REL::ID(OG, NG, AE)` | An explicit ID is supplied for OG, NG, and AE. |

The order never changes:

```cpp
REL::ID(
    2229323  // AE, and also used by NG
);

REL::ID(
    1546751,  // OG
    2229323   // NG and AE
);

REL::ID(
    1546751,  // OG
    2229323,  // NG
    2229323   // AE
);
```

Use one ID only after it has been tested on every runtime supported by the
plugin. If automatic OG or NG resolution is unavailable, use the two- or
three-ID form and provide the missing ID explicitly.

## Runtime-aware offsets

An ID normally identifies a function. A hook may need a location inside that
function. `REL::VariantOffset` selects an offset for the active runtime family:

```cpp
REL::Relocation<std::uintptr_t> hookSite{
    REL::ID(1546751, 2229323),
    REL::VariantOffset(ogOffset, ngOffset, aeOffset)
};
```

Two values mean `OG, modern`, with the modern value used by both NG and AE.
Three values mean `OG, NG, AE`.

## Automatic callsite discovery

Fixed interior offsets can move after an executable update. If a hook targets a
call from one known function to another, CommonLibF4RD can locate that call:

```cpp
constexpr REL::ID owner{ 1546751, 2229323 };
constexpr REL::ID target{ 881215, 2231148 };

REL::Relocation<std::uintptr_t> hookSite{
    owner,
    REL::VariantOffset{
        REL::AUTO_CALLSITE(target)
    }
};
```

The default form requires one unique call. First, last, and nth selectors are
also available for functions that intentionally call the target more than once:

```cpp
REL::AUTO_CALLSITE_FIRST(target);
REL::AUTO_CALLSITE_LAST(target);
REL::AUTO_CALLSITE_NTH(target, 2);
```

Always validate the intended call context before writing a hook. Automatic
discovery makes offsets more resilient; it does not replace ABI validation.

Compilable versions of these examples are in
`src/RelocationExamples.cpp`. They are deliberately not called by the plugin.

## Optional diagnostics

Create an empty file next to the DLL to enable a diagnostic mode.

### Requested IDs only

```text
F4RDExamplePlugin.trace
```

On the next launch, the file is overwritten with the IDs requested by this
plugin, their resolved RVAs, and any selected fixed or automatic offsets.
Remove or rename the file to disable tracing.

The untouched example does not execute its relocation examples, so its trace
can contain no ID entries until a validated plugin feature requests a
relocation.

### Complete database mapping

```text
F4RDExamplePlugin.mapping
```

On the next launch, the file is overwritten with a complete `ID RVA` mapping
for the current executable. Entries that cannot be resolved safely are written
to:

```text
F4RDExamplePlugin.mapping.fail
```

Complete mapping is intended for dedicated development and validation runs. It
can take noticeable time and should not be included in a normal plugin release.

## Turn this into your plugin

1. Rename `F4RDExamplePlugin` in `CMakeLists.txt`.
2. Update the version and package metadata.
3. Rename the C++ namespace if desired.
4. Replace the demonstration IDs with IDs required by the plugin.
5. Initialize features only after their required relocations are validated.
6. Test every supported runtime family with a `.trace` file.
7. Package the DLL and required assets, but not PDB or diagnostic marker files.

See the full
[CommonLibF4RD feature guide](https://github.com/Zzyxz/CommonLibF4RD/blob/main/docs/FEATURES.md)
for resolver status values, all callsite selectors, and additional examples.

## License

This example is available under the MIT License. See [LICENSE](LICENSE).
