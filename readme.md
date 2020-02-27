
# The requirements are

* CMake 3.11 or better; 3.14+ highly recommended.
* A C++17 compatible compiler
* Git
* Doxygen (optional)

To configure:

```bash
cmake -S . -B build -GNinja
```

To build:

```bash
cmake --build build

or

ninja.exe -C build
```

To test (`--target` can be written as `-t` in CMake 3.15+):

```bash
cmake --build build --target test
```

To build docs (requires Doxygen, output in `build/docs/html`):

```bash
cmake --build build --target docs
```
