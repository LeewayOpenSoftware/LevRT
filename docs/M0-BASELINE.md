# M0 baseline (Windows 11 26100 host, NOT LiberNT yet)

**Compiled:** `tools/levrt-activate/levrt-activate.cpp` and `tests/hstring_tests.cpp` with llvm-mingw clang 24.0.0git (`-std=c++17 -static -D_WIN32_WINNT=0x0A00`, `-lruntimeobject`). Both build without warnings that matter. No build system yet beyond that command line (CMake comes with the first LiberNT integration).

**Tested on the Windows 11 host only** (reference behaviour; LiberNT run is the next step and needs a VM slot):
- `hstring_tests`: 30 checks, 0 failed after one correction: a test assumed `WindowsCreateStringReference` rejects a source not NUL-terminated at `length` (E_INVALIDARG); Windows returns S_OK, so the check was removed (rule: Windows behaviour beats our reading of the docs).
- `levrt-activate`:
  - `Windows.Foundation.Collections.PropertySet` → `RoActivateInstance` S_OK, runtime class name echoed, TrustLevel 0 (BaseTrust), 5 IIDs (includes `IMarshal`-family `{00000038-...}` last).
  - `Windows.Data.Json.JsonObject` → S_OK, 7 IIDs.
  - `Windows.Foundation.PropertyValue` → `RoActivateInstance` fails `0x80040111` (CLASS_E_CLASSNOTAVAILABLE): it is a **static-only class** (activation factory only). `--factory` mode works (S_OK). So the design doc's M0 demo line "levrt-activate PropertyValue" should use `--factory`.

These outputs are the expected-result baseline to diff against when the same binaries run on LiberNT.
