# Spike A: xlang `meta_reader` vs Wine `rometadata`

Upstream: microsoft/xlang `e8a68e7c8a41801594cabca65ebd5d08ece05b75`, `src/library` (MIT, repo-level LICENSE; files have no per-file license header, see `licscan-xlang-e8a68e7.csv`).
Imported afterwards in the M1 step (see PROVENANCE.md).

## Result (compiled and run, separate)
- **Compiled:** yes, with llvm-mingw clang 24.0.0git, `-std=c++17 -static`. Two portability fixes are needed, neither in the headers' logic: add `#include <algorithm>` (missing in `view.h`/`column.h`: `std::equal_range`, `std::upper_bound`), and define `_WIN32_WINNT=0x0A00` so `CreateFile2` is declared. We do these via `-include algorithm -D_WIN32_WINNT=0x0A00`, no source patch required.
- **Tested (smoke only):** `spikes/A/spikeA.cpp` opens `.winmd` files and walks namespaces/types/methods. Run on four metadata files already on the host (read in place as test input; **nothing copied into the repo**): Foundation 5 ns / 169 types / 799 methods; Storage 10 / 373 / 2047; UI 40 / 1638 / 7785; ApplicationModel 57 / 1941 / 8905. All exit 0. Not validated against a reference count (no second reader run).
- **Wine rometadata** (`dll/win32/rometadata`, ~2.4k lines LGPL): implements the COM `IMetaDataDispenser/Import` surface over tables, not a typed model; no signature decoding API comparable to xlang's `cache`/`TypeDef` views.

## Decision (spike A)
Use **xlang `meta_reader`** as the single metadata reader inside LevRT (rule 4: one reader). Keep Wine `rometadata` only as the COM-facing `IMetaData*` shim, implemented on top of the same reader later if needed. Needs a `PROVENANCE.md` row at import time (verbatim, two build flags).
