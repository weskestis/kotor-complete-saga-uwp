# KOTOR: The Complete Saga — Xbox UWP

A non-commercial experimental native Xbox Series X Developer Mode host for *Star Wars: Knights of the Old Republic* and *Knights of the Old Republic II*.

## Status — Checkpoint 1D handoff

- Existing local CP1D source archive: `KOTOR_Complete_Saga_CP1D_Source.zip` (version 0.1.3).
- Rendering host: Direct3D 11 / x64 UWP proof of concept, **not** KOTOR gameplay.
- Graphics menu target: 720p through 4K, frame cap up to 120 FPS. Actual performance not measured on Xbox.
- Windows UWP compilation and Xbox deployment: **not yet verified**.
- No proprietary KOTOR game files included.

The original CP1D archive was created in ChatGPT; it must be imported into this repository before native packaging can be verified. See [the import instructions](docs/IMPORT_CP1D.md).

## Goals

1. Verify native UWP x64 host compilation in GitHub Actions and install the development-signed package on an Xbox Series X in Developer Mode.
2. Integrate a legitimate resource loader and executable KOTOR scene via the open-source [reone](https://github.com/modawan/reone) engine.
3. Build two separated campaign modes: Complete Saga and Expanded Galaxy, sharing a common engine and resources.
4. Add graphics presets and selectable resolution / frame caps up to 4K / 120 FPS **when supported**.

## CI

The [UWP x64 build workflow](.github/workflows/uwp-x64.yml) builds the source when the archive is imported. The workflow reports an informative failure if the required solution has not yet been uploaded. This distinguishes a missing project from a compiler error.

## Copyright and licensing

KOTOR 1 and 2 game assets are proprietary and must be supplied separately by players who own the games. Engine code derived from reone must retain its GPL-3.0-compatible terms and required attributions. No redistributable game assets, installers, credentials, or private signing keys belong in this repository.
