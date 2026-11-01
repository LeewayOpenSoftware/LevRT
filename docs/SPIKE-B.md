# Spike B: can WinUI 3 `xcp` core / controls be built or lifted with clang/mingw?

Upstream: microsoft/microsoft-ui-xaml `8da148dcd8ed65ea8b8ae664fbe45c9cc82e372b` (main). Sparse partial clone of `dxaml/xcp/{core,pal,plat,win,inc,common,components,control}` and `controls/dev` plus root files: 249 MB on disk (limit was 1.5 GB). Nothing is imported into LevRT.

## What was measured
- **Size/structure (GitHub tree):** `dxaml/xcp/core` 1,435 files / 15.9 MB (Parser 45k lines incl. `MeScanner`, `ObjectWriter`, and the **XBF binary reader** `XamlBinaryFormatReader*.cpp`; animation, text, input, layout, compositor), `dxaml/xcp/dxaml` (27.9 MB, the WinRT-facing layer), `controls/dev` (110 MB, control templates and logic). `dxaml/test/resources` (216 MB) is test data, not needed.
- **Licensing:** 5,819 files scanned: 5,511 with explicit MIT header, 307 repo-level, 1 aggregate NOTICE (see `NUGET-AUDIT.md`).
- **Compile attempt (compile only, with our clang/mingw):** `clang++ -fsyntax-only` on `core/Parser/MeScanner.cpp` and `LineInfo.cpp` fails immediately in the toolchain's own `Windows.Foundation.h` ("redefinition of IReference<unsigned char>") because every file includes `precomp.h`, which pulls `<windows.foundation.h>`, `"Microsoft.UI.Xaml.h"` (a **MIDL-generated header** from the Windows App SDK/WinUI IDL, not in the repo), `<wil\Result.h>`, `xcpwindows.h`, the compositor stack, and FrameworkUdk. A sample build was not possible without those inputs. Test status: none (nothing to run).

## Findings
1. The core is the XAML "xcp" engine: it is **tightly coupled** to (a) the WinRT-generated `Microsoft.UI.Xaml.h` and Windows SDK WinRT headers, (b) the Windows composition stack (`dcomp` in 24 files, `Windows.UI.Composition` in 12, d3d11 in 12, d2d1 in 6, dwrite in 11) and (c) closed packages `Microsoft.FrameworkUdk` / `WinUIDetails` (see `NUGET-AUDIT.md`).
2. Build system is MSBuild/vcxproj/NuGet; `experimental-clang.props` exists but is untested by us. A direct build on our toolchain is **not feasible without** generating the WinRT headers and providing the FrameworkUdk surface.
3. Positive: Parser, ObjectWriter, animation math, text and layout logic are plain C++ and MIT-headed. The **XBF reader is in MIT source**: the binary XAML format is therefore documented by public MIT code (relevant to the owner's earlier `.xbf` question: implementing a reader from MIT source is a different legal position than reverse-engineering Windows files; owner to confirm).

## Go/No-go
- **NO-GO** for "import and build the xcp core / WinUI product as-is" (blockers: generated headers, FrameworkUdk, WinUIDetails, composition coupling; est. months of decoupling before first compile).
- **GO** for **leaf-module extraction** into LAML, one module at a time, each with its own PROVENANCE row and a decoupling shim: (1) XBF + XAML parser (`core/Parser`) once a replacement type-info provider exists, (2) animation/easing math, (3) text/layout algorithms, (4) control templates from `controls/dev` (XAML/resource files are data, MIT-headed). Estimated effect on the design: M3 (parser) likely shortened since a reference implementation can be adapted module-wise, M4/M5 unchanged because composition/render and binding glue must be ours anyway; the earlier 14-20 engineer-month range stays; revisit after the first extracted module compiles.
- Needs owner decision: (a) confirm the strategy "extract leaf modules, do not port the product", (b) `.xbf` reader from MIT source acceptable?
