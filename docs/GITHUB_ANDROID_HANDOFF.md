# GitHub Windows build handoff (Android-only workflow)

This repository does not automatically exist on GitHub. The archive is a ready-to-upload source tree. No changes were made to other personal GitHub repositories.

**Purpose:** build a native UWP development probe using GitHub-hosted Windows instead of a local PC.

1. Create a GitHub repository for this project (e.g. `kotor-complete-saga-uwp`). An initialized default branch with a starter README is fine; replace that starter README with the source README when uploading the archive contents.
2. Upload all extracted folder contents, **including the hidden `.github/workflows` directory**, to repository root. Mobile browsers may need desktop site enabled and a file-management app to extract the archive. The `KOTOR_Complete_Saga_CP1C/` folder itself must not be an extra wrapper at the repository root.
3. Open **Actions → Xbox CP1C UWP x64 build and development package → Run workflow**, or let the push trigger it.
4. If Windows compilation fails, download or copy the log. Do not claim Xbox readiness. Fix the first compiler error and rerun.
5. If it succeeds, download artifact `KotorSaga-CP1C-x64-dev-signed`; it should contain `.appx` or `.msix` plus `KotorSagaDev-PUBLIC.cer` (and possibly framework dependencies). Inspect signature and ensure the Xbox Dev Mode device accepts the package/certificate before calling it installable.
6. On Xbox, verify reported *display buffer* vs *internal texture*; test menu/back behavior, frame caps and save/restart. Save the actual video outputs and crash logs. Do not call the game playable: only a diagnostic 2D GPU scene exists.

The default certificate is generated anew at each build, which may require renewed device trust on updates. For stable releases, provision a stable private development signing certificate separately via a secure secret. **Never commit or share a `.pfx` file**.

Xbox device resources and display modes can differ from the retail console game environment. Choose a conservative first boot 1080p60. Start 4K and 120 testing only after basic startup.
