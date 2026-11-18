# Provenance

Every component imported into LevRT from another project must be listed here,
with its license and with its copyright and license notices preserved in the
imported files. A component that is not listed here must not be in the tree.

Rules:

- Record the exact upstream commit (or release tag) the files were taken from.
- Keep the upstream copyright and license headers in every imported file.
- Describe every local modification; an unmodified import says "none".
- Add the component's required notices to [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).
- Only clean-room sources are acceptable: public documentation, Wine, ReactOS
  and other free-software projects under compatible licenses.

| Component | Upstream URL | Commit / tag | License | Files | Modifications | Imported by / date |
|-----------|--------------|--------------|---------|-------|---------------|--------------------|
| xlang `meta_reader` | https://github.com/microsoft/xlang | `e8a68e7c8a41801594cabca65ebd5d08ece05b75` (master, 2026-11-01) | MIT (root LICENSE, Copyright (c) Microsoft Corporation; files have no per-file header, see docs/licscan-xlang-e8a68e7.csv) | `third_party/xlang/library/meta_reader.h`, `impl/base.h`, `impl/meta_reader/*.h` (19 files, listed in third_party/xlang/SHA256SUMS.txt); `third_party/xlang/LICENSE` | One downstream portability fix: `row_base::operator--()` now returns the derived row reference, matching `operator++`; this is not byte-identical to upstream. CMake force-includes `algorithm` and `utility` and defines `_WIN32_WINNT=0x0A00`. Detours and all other xlang files excluded | Meredith (Leeway Open Software), 2026-11-01; portability fix added by Oscar, 2026-11-18 |
