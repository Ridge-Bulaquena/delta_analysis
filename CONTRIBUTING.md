# Contributing

## Quick start
1. Configure: `cmake --preset x64-debug-linux`
2. Build: `cmake --build out/build/x64-debug-linux -j`
3. Test: `ctest --test-dir out/build/x64-debug-linux --output-on-failure`

## Style
- C++20 required.
- Keep headers self-contained.
- Prefer unit tests for bug fixes and new features.

## Pull requests
- Keep PRs small and focused.
- Include test evidence in PR description.
