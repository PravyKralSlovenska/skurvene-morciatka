# skurvene-morciatka

- maturitny projekt 2026

# FLAME CHART

```bash
perf record -F 99 -g ./morciatko
perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
firefox flame.svg
```

# BUILD NA LINUX

```bash
cmake -S . -B build -DUSE_BUNDLED_DEPS=OFF -DBUILD_TESTS=ON
cmake --build build --parallel --target morciatko morciatko_tests
ctest --test-dir build --output-on-failure
./build/morciatko
```

# BUILD NA WINDOWS

```powershell
cmake -S . -B build -DUSE_BUNDLED_DEPS=ON -DBUILD_TESTS=ON
cmake --build build --config Release --parallel --target morciatko morciatko_tests
ctest --test-dir build -C Release --output-on-failure
.\build\Release\morciatko.exe
```

# JEDEN BUILD FOLDER

Pouzivaj stale iba priecinok `build`.

- Ked buildis Linux system deps: `-DUSE_BUNDLED_DEPS=OFF`
- Ked buildis Windows/bundled deps: `-DUSE_BUNDLED_DEPS=ON`

Pri prepnuti medzi rezimami v rovnakom `build` priecinku urob jeden z krokov:

```bash
# varianta A: cisty rebuild
rm -rf build

# varianta B: vymazat iba CMake cache
rm -f build/CMakeCache.txt
rm -rf build/CMakeFiles
```
