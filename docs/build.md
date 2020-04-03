
# The requirements are

- CMake 3.17 or better ;-) for multi-config generator.
- A C++17 compatible compiler
- Git
- Doxygen (optional)

To configure:

```bash
cmake -S . -B build -GNinja

cmake -S . -B build-multi -G"Ninja Multi-Config"

cmake -S variant/fusionTX -B build-fusionTX -GNinja
```

To build:

```bash
cmake --build build

or

ninja.exe -C build

ninja -C build-multi -f build-Release.ninja
```

To test (`--target` can be written as `-t` in CMake 3.15+):

```bash
cmake --build build --target test
```

To build docs (requires Doxygen, output in `build/docs/html`):

```bash
cmake --build build --target docs
```
