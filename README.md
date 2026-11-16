# KOTOR: The Complete Saga — Xbox UWP

A non-commercial experimental native Xbox Series X Developer Mode host for *Star Wars: Knights of the Old Republic* and *Knights of the Old Republic II*.

## Status — Checkpoint 1D handoff

- CP1D version 0.1.3 source code is committed to this repository (`src/`, `xbox/`, `ci/`, `docs/`, `tests/`).
- Rendering host: Direct3D 11 / x64 UWP proof of concept, **not** KOTOR gameplay.
- Graphics menu target: 720p through 4K, frame cap up to 120 FPS. Actual performance not measured on Xbox.
- Windows UWP x64 compilation, development signing, and package verification: **passed on GitHub Actions** ([successful run](https://github.com/weskestis/kotor-complete-saga-uwp/actions/runs/43778112398)). Xbox installation and hardware performance: **not yet tested**.
- No proprietary KOTOR game files included.

The CP1D source has been imported, including the original build scripts and sample imagery. The CI runs exposed and helped fix a Direct2D factory compilation issue, missing UWP manifest metadata, and package verification of framework dependencies. The successful CI artifact includes the MSIX, public development certificate and VCLibs dependencies.

## Goals

1. Verify native UWP x64 host compilation in GitHub Actions and install the development-signed package on an Xbox Series X in Developer Mode.
2. Integrate a legitimate resource loader and executable KOTOR scene via the open-source [reone](https://github.com/modawan/reone) engine.
3. Build two separated campaign modes: Complete Saga and Expanded Galaxy, sharing a common engine and resources.
4. Add graphics presets and selectable resolution / frame caps up to 4K / 120 FPS **when supported**.

## CI

The [UWP x64 build workflow](.github/workflows/uwp-x64.yml) compiles, development-signs and verifies an Xbox UWP x64 package, then uploads build artifacts. Follow [GitHub Actions runs](https://github.com/weskestis/kotor-complete-saga-uwp/actions) for the current status. The Linux workflow independently validates the portable code. A successful build is not proof the package boots on an Xbox.

## Copyright and licensing

KOTOR 1 and 2 game assets are proprietary and must be supplied separately by players who own the games. Engine code derived from reone must retain its GPL-3.0-compatible terms and required attributions. No redistributable game assets, installers, credentials, or private signing keys belong in this repository.
