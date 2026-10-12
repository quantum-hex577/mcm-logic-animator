# ChainLab: C++ Matrix Chain Multiplication Logic Animator

ChainLab is a dependency-free C++17 terminal program for exploring the bottom-up dynamic-programming solution to Matrix Chain Multiplication (MCM). Rather than displaying only the final answer, it generates a sequence of immutable frames that show the state of the cost table, the split table, the candidate being evaluated, the active pseudocode line, and the traceback tree.

## Features

- Accepts 3-9 positive integer dimensions, representing a chain of 2-8 matrices.
- Rejects non-positive dimensions and values greater than 10,000.
- Shows the minimum-cost table `m` and selected-split table `s` at every frame.
- Generates frames for base cases, interval initialization, each split candidate, each improved minimum, interval finalization, and traceback.
- Highlights the active table cell and the candidate's referenced subproblems in ANSI-capable terminals.
- Supports next, previous, autoplay, reset, jump-to-frame, speed adjustment, and jump-to-final-result controls.
- Animates construction of the optimal parenthesization tree in the terminal.
- Compares the optimum scalar multiplication count with fixed left-to-right evaluation.
- Uses standard C++17 only; no third-party libraries or network access are required.

This is a terminal-based C++ application, not the previous browser/React interface. The algorithmic behavior is retained, but the display uses text tables and terminal highlighting instead of a graphical browser layout.

## Requirements

- A C++17 compiler (GCC, Clang, or a recent MSVC)
- Optional: CMake 3.16+ to use the provided build file

## Compile with g++

From the project root:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp src/mcm.cpp -o chainlab
./chainlab
```

Windows MinGW:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp src/mcm.cpp -o chainlab.exe
.\chainlab.exe
```

If ANSI coloring/clearing looks odd in your terminal, run `chainlab --no-color --no-clear` (or `chainlab.exe --no-color --no-clear`).

## Build with CMake

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

The built executables are `chainlab` and `chainlab_tests` (on Windows, typically `.exe`). The `docs/` directory contains the editable project report and its PDF export.

## Run the tests without CMake

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Isrc tests/tests.cpp src/mcm.cpp -o chainlab_tests
./chainlab_tests
```

Expected output:

```text
All 5 C++ regression test groups passed.
```

## Using the animator

At startup, enter matrix dimensions as space- or comma-separated integers. For example:

```text
30 35 15 5 10
```

This represents `A1 = 30x35`, `A2 = 35x15`, `A3 = 15x5`, and `A4 = 5x10`. Press Enter on the initial prompt to use this example.

Controls:

- `n` — next frame
- `b` — previous frame
- `a` or `p` — autoplay remaining frames; use Ctrl+C to interrupt
- `g` — jump to a numbered frame
- `r` — reset to the first frame
- `s` — change autoplay delay (50-2000 milliseconds)
- `f` — jump to the final result
- `h` — help and display legend
- `q` — quit

Cyan brackets mark the active table cell, and yellow braces mark cells used in a candidate cost (when terminal color is enabled).

## Algorithm

For dimensions `p[0..n]`, matrix `Ai` has dimensions `p[i-1] x p[i]`. The DP recurrence is:

`m[i,j] = min(m[i,k] + m[k+1,j] + p[i-1] * p[k] * p[j])`, for `i <= k < j`, with `m[i,i] = 0`.

The core algorithm uses `O(n^3)` time and `O(n^2)` storage for the cost and split tables. The animation implementation additionally stores table snapshots per frame; that intentionally trades memory for straightforward backward navigation. The input is capped at eight matrices to keep these snapshots and terminal output manageable.

## Project structure

```text
src/
  main.cpp       Terminal UI, tables, pseudocode panel and navigation
  mcm.cpp        MCM recurrence, immutable frame generation and traceback
  mcm.hpp        Data structures and public declarations
tests/
  tests.cpp      Regression tests for known results and input validation
CMakeLists.txt   Optional CMake build and test setup
.github/workflows/ci.yml   Automated GitHub build and test workflow
docs/            Editable report and PDF report
PUBLISHING.md    Repository setup and submission checklist
README.md        Setup, controls, algorithm and project limitations
LICENSE          MIT license
```

## Academic limitations

The baseline measures the theoretical number of scalar multiplications, not wall-clock runtime. The project has not been evaluated in a controlled student study, and no educational-effectiveness claim is made. The code has been compiled and tested locally using the commands above; verify it again using your department's compiler environment before submission.

## References

1. T. H. Cormen, C. E. Leiserson, R. L. Rivest, and C. Stein, *Introduction to Algorithms*, 3rd ed. MIT Press, 2009, Chapter 15.
2. C. D. Hundhausen, S. A. Douglas, and J. T. Stasko, “A Meta-Study of Algorithm Visualization Effectiveness,” *Journal of Visual Languages & Computing*, vol. 13, no. 3, pp. 259-290, 2002, doi: 10.1006/jvlc.2002.0237.
3. J. Urquiza-Fuentes and J. A. Velazquez-Iturbide, “A Survey of Successful Evaluations of Program Visualization and Algorithm Animation Systems,” *ACM Transactions on Computing Education*, vol. 9, no. 2, article 9, 2009, doi: 10.1145/1538234.1538236.
