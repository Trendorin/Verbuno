# Contributing

Contributions that keep Verbuno small, native and explicit about its privacy boundary are welcome.

## Before opening a change

1. Search existing issues and avoid duplicate work.
2. Keep the C++20 and Qt 6 Widgets architecture; do not introduce Electron, QML, a hosted backend or an SDK when Qt Network is sufficient.
3. Never add telemetry, silent network requests, plaintext credential storage or provider claims that cannot be verified.
4. Update tests and all affected documentation.

## Build and test

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

For memory-safety checks:

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
```

Run `clang-format` on changed C++ files. Keep commits focused and use factual commit messages.

## Pull requests

Describe the user-visible change, privacy or security impact, desktop environments tested and commands used for validation. Do not attach real API keys or private translation samples.
