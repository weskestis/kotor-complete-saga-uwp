# Checkpoint 1D — Windows build and Xbox Dev Mode deployment

This checkpoint contains **source**. It does **not** contain a compiled `.appx`/`.msix`; compilation requires Windows MSVC UWP tools and must succeed before deploying. The package displays a Direct3D 11 diagnostic scene, **not** any Knights of the Old Republic game content.

## Phone-only path using GitHub Actions

1. Create a repository in your GitHub account, e.g. `kotor-complete-saga-uwp`. The current project has **not** been pushed automatically; if you are reading this ZIP, it is a local artifact.
2. Upload the *contents of this archive's top-level project folder* to the repository root, preserving `.github/workflows/uwp-x64.yml`, `ci/`, `src/`, `xbox/`, and `tests/`. Do not upload the ZIP as the only repository file; Actions needs the actual source tree. GitHub browser upload has file-count/size restrictions; a local Git client or an authorized connector may be more practical.
3. In your GitHub repository, open **Actions → KOTOR Saga CP1D / Xbox UWP x64 package → Run workflow**.
4. The runner first compiles the UWP x64 project. It creates a one-time development signing certificate and, **if** compilation/package verification succeeds, produces the `KotorSaga-CP1D-Xbox-UWP-x64` artifact containing `.appx` or `.msix` plus `KotorSagaDev-PUBLIC.cer`.
5. If compilation fails, inspect the `KotorSaga-CP1D-Windows-Build-Logs` artifact (`msbuild.binlog`, `build-transcript.txt`). Do not assume failure means the console is incompatible; the Windows SDK or package build may be the problem.
6. On Android, download the generated GitHub Actions artifact ZIP and extract the `.appx` and `.cer`. Open **Xbox Dev Mode → Dev Home**, ensure the Xbox and phone share a network, and use the Dev Home Device Portal address in your mobile browser.
7. Under the portal's Apps installation interface, select the package, the exported public signing certificate when required, and any declared framework dependencies. Deploy to Xbox. Record the first-run result.

> Each CI run creates a different self-signed **development** certificate. For reliable in-place updates, establish a stable signing identity/certificate *privately*, never upload `.pfx` to a public repo, and store secrets only through protected CI settings. Do not download the runner's private signing key.

## First Xbox acceptance test

- Console opens the **KOTOR Saga CP1D Render Probe** without returning instantly to Dev Home.
- Green/dark animated grid and the status overlay appear.
- MENU opens the menu, D-pad navigates, A changes a value, B returns to the scene **without exiting**.
- Start with **1080p / 60 FPS**, then try **1440p**, **2160p**, and a **120 FPS target**. Observe the *actual backbuffer*, *allocated internal* size, and *measured* frame rate. Do not infer 4K/120 from the selected menu values.
- If the app aborts after activation, use the Xbox Device Portal's app file browsing to retrieve the app's `LocalState/diagnostics.log` and send the latest lines with the Windows build logs.
- Verify switching away and returning to the app preserves navigation and settings.

**Xbox testing not completed in this checkpoint.** 4K output, high refresh, and SDK support require live device evidence. No audio, KOTOR assets, NPCs, dialogue, or combat are included.

Useful official docs:
- https://learn.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal
- https://learn.microsoft.com/en-us/windows/uwp/apps-for-xbox/development-options
- https://learn.microsoft.com/en-us/windows/uwp/gaming/plan-your-directx-port
