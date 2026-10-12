# Movement-speed design benchmark

A small, dependency-free C++20 experiment comparing equivalent movement updates:

1. **Virtual provider** — each entity asks a polymorphic `SpeedProvider`.
2. **Plain function** — an entity stores its speed and a free function updates it.
3. **Struct data (SoA)** — positions and speeds are contiguous, separate arrays.
4. **Tagged switch** — a closed set of movement kinds, included as a practical middle ground.

All models use speeds `2`, `6`, and `12`, repeat in the same order, and update `x += speed * dt`. The test proves this agreement before you put weight on timing results.

## Build on Windows

Use a **Release** build; Debug timings are not meaningful.

### Visual Studio 2022 Developer PowerShell

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\movement_bench.exe
```

### Ninja (MSVC, clang-cl, or MinGW)

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
.\build\movement_bench.exe --entities 1000000 --iterations 100 --samples 9
```

Use `--help` to see available benchmark options. Keep the machine mostly idle, use the same executable and input across comparisons, and run several independent invocations. The displayed median is more useful than the single fastest result; the range exposes noisy runs.

## What is—and is not—being measured

The timed region contains only repeated movement updates. Allocation, world construction, copying/resetting the world, printing, and checksum calculation occur outside it. A volatile checksum makes the final positions observable, so the compiler cannot simply discard the updates.

This can demonstrate the cost of *this exact data layout, dispatch pattern, compiler, CPU, and workload*. It cannot prove that virtual interfaces are universally bad, that a class design is wrong, or that the fastest microbenchmark wins in a real program. In particular:

- The virtual case uses one provider allocation and pointer chase per entity; that intentionally captures a common polymorphic-entity layout, not every possible virtual design.
- The tagged case has predictable repeating kinds. Shuffle kinds or use a distribution matching your game/application if branch prediction matters.
- SoA benefits most when the update needs only selected fields. If later code needs all of an entity together, an array-of-structs layout may be better.
- Real performance often turns more on cache locality, allocations, batching, synchronization, I/O, and algorithmic work than on the call syntax itself.

Treat a surprising result as a prompt to inspect generated code and profile the complete workload, not as a design verdict.

## Easy experiments

- Change the three speeds, number of kinds, and their distribution.
- Put two or more behaviors on `SpeedProvider` to see whether its extensibility earns its cost.
- Add fields that a real update reads/writes, then measure again.
- Compile with a second compiler and compare *trends*, not absolute nanoseconds.
