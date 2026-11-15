# CP1C Xbox acceptance checklist

Do not mark as pass without hardware evidence.

## Compilation

- [ ] Visual Studio 2022 x64 UWP compilation on Windows runner succeeds.
- [ ] Appx manifest validates and packaging produces x64 UWP package.
- [ ] Package signature successfully created using a development certificate; public .cer exported.
- [ ] Xbox Dev Mode publisher trust and any UWP dependency packages confirmed.
- [ ] Xbox Dev Mode accepts and launches package.

## Native graphics probe

- [ ] D3D11 hardware device initializes.
- [ ] Flip-sequential swap chain creates for the Xbox CoreWindow.
- [ ] Back buffer size reported accurately.
- [ ] Offscreen target allocated independently (720p/1080p/1440p/2160p), with actual allocation size shown.
- [ ] Switching internal scale changes the GPU scene target while preserving swap-chain output resolution.
- [ ] Target allocation failure falls back to a smaller buffer and reports that buffer rather than falsely claiming 4K.
- [ ] Overlay text is readable at 1080p, 1440p and 4K HDMI output modes (where supported).
- [ ] Controller MENU opens/returns; B returns to status without terminating application.
- [ ] D-pad moves selection; left/right changes values; A changes selected value.
- [ ] Settings persist after suspend and full relaunch.
- [ ] Measured frame-loop values at 30, 60, 90, 120 caps recorded; do not interpret as output refresh until independently verified.
- [ ] Resize/resume/device-loss paths do not crash.
- [ ] FPS overlay reports reasonably on target hardware; verify against independent diagnostics.

## Deliberately not acceptance items for CP1C

- Actual KOTOR 3D scene or demonstrated 4K/120 performance — not implemented.
- Loaded KOTOR scene, audio, models, dialogues, or combat — not implemented.
- KOTOR 1/KOTOR 2 merge — not implemented.
- Gameplay visual quality preset application — not implemented.

## Bug reporting format

Record: test action, expected vs. actual result, whether Xbox is in 4K 60 or 4K 120 display mode, controller type, device firmware, screenshot if helpful, Windows/GitHub build log, and the Xbox Dev Portal app crash logs (if any).

Never include private keys, Microsoft-account authentication details, or copyrighted game assets in public logs.
