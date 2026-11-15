# Xbox-native implementation checklist (design, NOT completed code)

## Targets

- Native x64 Xbox Series X application using UWP APIs and Direct3D; **no emulation or game streaming**.
- Output resolution: detect the actual HDMI capability; render scene independently at any chosen target up to native 3840x2160.
- Current **FPS cap** is in software settings (24–120). Do not claim 120 displayed frames on a 60 Hz TV. For 120 Hz output, the console and connected display must negotiate a 120 Hz mode; refresh rate != achieved FPS.
- Start performance testing at 720p60, then 1080p60, 1440p60, 4K60, 4K120; report GPU/CPU times. Avoid choosing 4K120 by default until benchmark results prove it stable. Present saved settings separately from the recommended auto-detected preset.
- Use high-quality UI rendered at display resolution while scaling 3D scene resolution independently.
- VSync on/off, frame cap, presentation interval and display mode must be designed together. 90 FPS on a fixed 120 Hz output cannot be guaranteed evenly paced; VRR depends on the display and console support. Never bypass 120 maximum software frame target.

## Reone adapter mapping

Source: https://github.com/modawan/reone/blob/master/doc/engine-options.md

Existing names: width, height, vsync, texquality, shadowres, anisofilter, drawdist, grass, shadows, particles, pbr, ssao, ssr, fxaa, sharpen. `reoneOptions()` writes these known names for use *after* a backend exists.

Not currently known as reone options: fps_cap, fps_overlay, adaptive_resolution. Those are **host features that must be implemented**. The existing `shadowres` formula is 2^(10+N), so 512 is not representable and is mapped to the lowest supported 1024 setting with shadows disabled in the Low preset.

## Native renderer work

- CP1C source creates a D3D11/DXGI swap chain and a separate scene render target at the selected internal resolution, with GPU texture allocation fallback. The actual Xbox output and runtime stability remain unverified. Port KOTOR 3D rendering in later phases.
- Audit all GLSL, OpenGL state, draw calls, shader includes and texture formats; create HLSL equivalents and compatible buffers/pipelines.
- Resource loader must work in UWP app sandbox and with imported owned data. Account for differing KOTOR 1 vs 2 format/lookup behavior.
- Serialize settings to UWP `LocalState` (atomic save); do not mix saves or override content with config changes.
- Controller navigation should allow all settings using Xbox gamepad, including an Apply/Keep/Revert timeout when switching video modes.
- GPU/CPU benchmarking: frame time histogram, 1% lows, actual refresh, output mode, scene size, shader compilation/hitches.

## Research references

- Microsoft's UWP scaled swapchain guidance: https://learn.microsoft.com/en-us/windows/uwp/gaming/multisampling--scaling--and-overlay-swap-chains
- Microsoft DXGI display mode enumeration: https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgioutput1-getdisplaymodelist1
- Reone project / architecture: https://github.com/modawan/reone
