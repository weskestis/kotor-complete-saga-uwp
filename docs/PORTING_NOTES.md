# Engine adapter and native UWP constraints

## What CP1B proves (after successful Xbox test)

An application can create a CoreWindow, a D3D11 hardware device and a native input/UI loop. It does **not** prove 4K/120, nor that reone or KOTOR game data can be run on this platform.

## Future render pipeline

- Game logic and resources: adapter to reone C++ modules (review GPL obligations).
- Renderer: OpenGL 4.0 / GLSL -> UWP-compatible D3D11 / HLSL, shaders precompiled with Windows SDK tools.
- Game scene renders at selectable independent resolution: 640×360 through 3840×2160; actual output size from Xbox CoreWindow; physical display modes queried only where supported by platform APIs.
- Postprocessing and render texture sizes sized to user settings; cap GPU allocations, handle low-memory and lost-device conditions; do not exceed Dev Mode resource budgets.
- Input: Windows.Gaming.Input.Gamepad -> engine actions; keyboard optional; B navigates back in menus rather than exiting.
- Saving: app-isolated LocalFolder until a sandbox-safe game-data importer is added.

The current reone source was described as SDL3/OpenGL4.0-based and incomplete as of November 2026; its published game content remains separate and must be legally supplied by the user.

## Known integration risks

* UWP C++/CX compiler/Windows SDK and packaging settings need Windows CI verification before any build claim.
* UWP may not allow display refresh selection or 120 Hz output in Developer Mode; a 120 FPS user cap does not establish display capability.
* Full-resolution 4K buffer + dynamic shadows and multiple post effects may exceed practical Dev Mode budgets.
* Cross-game dialogue, scripts, model format differences, and save schema need compatibility work beyond graphics.
* The Xbox package is a native app shell, not an emulator nor a wrapper for Windows desktop executables.
