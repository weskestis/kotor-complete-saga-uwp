# Ordered project phases (Xbox first)

- [x] CP1A: Independent graphics settings model + parser + validation + portable tests.
- [x] CP1B: Native UWP x64 host SOURCE with D3D11 clear and gamepad-navigable diagnostics (Windows compile not verified).
- [x] CP1C: GPU offscreen render target SOURCE with independent internal resolution + CI dev-signing pipeline, and portable tests (Windows compile not verified).
- [ ] CP1D: Build and test the signed native UWP package through Windows GitHub Actions and on Xbox Dev Mode; resolve compiler/device failures.
- [ ] CP1E: Port first resource parser and load a user-owned KOTOR texture/model into a test scene.
- [ ] CP1F: Load a playable KOTOR room with actor movement, camera, collisions and frame profiling.
- [ ] CP2: Reone game/scene mechanics, KOTOR 1 dialogs/combat/saves, first campaign testing.
- [ ] CP3: KOTOR 2 content and rules, campaign select and separate save slots.
- [ ] CP4: Complete Saga storyline handoff; Expanded Galaxy alternate mode; no wholesale copyright asset redistribution.
- [ ] CP5: Optional progression-safe randomized mode and New Game+.

Every checkpoint: build validation, logs, rollback, a recoverable source archive and eventually signed MSIX. Do not represent project as playable before Xbox hardware confirmation.
