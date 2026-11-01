# LevRT and LAML: design (WinRT runtime and XAML-class UI for LiberNT)

Card `libernt-levrt-laml-design`. Design only, clean-room: public Microsoft docs, Wine/ReactOS sources, observable behaviour. No MS code, no disassembly, no `.winmd`/`.xbf` data copied from Windows into LiberNT. Implementation only after owner OK.

## 1-page summary

- **LevRT** = Leeway Runtime: our implementation of the Windows Runtime (WinRT) activation/metadata/type layer. **LAML** = Leeway Application Markup Language: our declarative UI layer, a XAML-compatible dialect with its own extension and namespace.
- **Branding rule (resolves the key constraint):** *brand lives in product names, docs, file extension, our own namespaces and private module names; the public ABI stays Microsoft-named.* A Windows app calls `RoActivateInstance(L"Windows.UI.Xaml.Controls.Button")`, links `combase.dll`, `api-ms-win-core-winrt-*`, and loads `Windows.UI.Xaml.dll` by that name. Those names cannot change. So: **DLL/export/class names stay `combase`, `wintypes`, `Windows.*`**; LevRT/LAML is the *project and implementation name* (docs, `levrt` source dirs, CMake targets, an internal `levrt.dll` core, version string, `.laml` files). Never rename an ABI symbol.
- **Reuse:** Wine `combase` (HSTRING, Ro* activation, IInspectable, error info), `wintypes` (2k-line real property/value set, maps, buffers, storage), `rometadata` (ECMA-335 table reader, 2.2k lines), `windows.ui` (corewindow, uisettings, ...), `windows.ui.xaml` (only ColorHelper today). ReactOS: GDI/user32/uxtheme/dwrite/gdiplus/windowscodecs/uiautomationcore. **Missing and needed:** `d2d1`, `d3d11/dxgi` (partial in tree?), `dcomp`, a real `Windows.UI.Xaml.dll`.
- **Shipping order (M0..M6):** M0 foundation tests, M1 activation via registry and `.winmd`, M2 the Windows.Foundation/Storage/Collections surface used by real binaries, M3 LAML parser + object tree (headless), M4 layout + GDI/DirectWrite renderer with a Button/TextBox/StackPanel demo, M5 data binding/styles/templates + a Settings-style demo app, M6 real UWP/WinUI-style app hosting (CoreWindow + AppX activation) as a stretch. Rough total 14 to 20 engineer-months for M0-M5; M6 open-ended.
- **Honest risk:** matching *Microsoft's* XAML (thousands of controls, `.xbf` binary format, VisualStateManager, composition, DirectManipulation) is the long tail. The design therefore delivers a **LAML-native toolkit first** (apps we own: Settings, tools), with a documented compat surface that grows toward `Windows.UI.Xaml`. We claim compatibility only for what the test suite proves.

---

## 1. Layering

```
 app (Windows binary or LiberNT app)
   |  RoInitialize / RoActivateInstance / RoGetActivationFactory / WindowsCreateString
 combase.dll  (ABI name kept)        <- LevRT core: HSTRING, apartments, activation, error origination
   |  activation catalog: registry + .winmd metadata + manifest (appx/sxs)
 per-namespace DLLs (ABI names kept)  <- Windows.Foundation(wintypes), Windows.Storage, Windows.UI, Windows.UI.Xaml ...
   |  each exports DllGetActivationFactory / DllCanUnloadNow
 LAML runtime (internal: levrt-laml)  <- markup parser, element tree, DP system, layout, styles, binding
   |  render backend
 renderer: GDI+/DirectWrite now -> D2D1 later -> DWM composition (dwmcore) optional
```

### 1.1 Foundation (combase, HSTRING, IInspectable)
Starting point: Wine `combase` (25k lines with mscoree) already has `RoInitialize`, `RoActivateInstance`, `RoGetActivationFactory`, HSTRING and `IInspectable` plumbing; many Ro* are FIXME stubs (`roapi.c`: `RoActivateInstance` is "semi-stub", error origination, `RoRegisterActivationFactories`, `RoGetServerActivatableClasses` stubbed). Work: finish these, no new DLL.

### 1.2 Activation registry
Public contract (docs.microsoft.com "Windows Runtime activation"): a class id (string) maps to a server DLL through `HKLM\SOFTWARE\Microsoft\WindowsRuntime\ActivatableClassId\<id>` (`DllPath`, `ActivationType`, `Threading`, `TrustLevel`) and via AppX manifests (in-proc server entries). LevRT implements the same keys (they are public and apps/installers write them), plus our own *optional* override key `HKLM\SOFTWARE\Leeway\LevRT\...` for LAML-private classes. Lookup order: manifest, ActivatableClassId, built-in table (the namespace DLLs we ship), `.winmd` namespace resolution (`RoResolveNamespace`).

### 1.3 Metadata (.winmd)
`.winmd` = ECMA-335 metadata (public standard). Reuse Wine `rometadata` (table reader) as the engine; add: type lookup by full name, interface/method signature parsing for `IInspectable` vtable layout, `RoGetMetaDataFile`, `RoParseTypeName`. **Metadata source rule:** the `.winmd` files LiberNT ships are **authored by us from public API documentation/headers (WinRT IDL in the SDK)**, compiled with our own MIDL/winmdidl-equivalent step (Wine `widl` already builds `.idl` for `windows.*`); we do not copy Microsoft's `.winmd`. Apps that carry their own `.winmd` (from their SDK) work as-is.

### 1.4 Language projections
Not our problem for M0..M5: C++/WinRT headers are header-only, the projection is compile-time; C# projection needs a CLR (Mono, parked until upstream). Native WinRT apps work with just the ABI.

### 1.5 Namespace DLLs
Keep what exists (`wintypes`, `windows.applicationmodel`, `.storage`, `.globalization`, `.devices.*`, `.media.*`, `.ui`, `.ui.core.textinput`, `.system.profile.*`). Add as needed by M2: `Windows.Foundation.Collections` (in wintypes), `Windows.Storage.Streams`, `Windows.Data.Json`, `Windows.UI.Core` (CoreWindow/CoreDispatcher in `windows.ui`), `Windows.System`. Each needs *tests against the public docs*.

## 2. LAML

### 2.1 What it is
A declarative UI dialect: elements, attributes, property elements, markup extensions (`{Binding}`, `{StaticResource}`, `{x:Bind}` later), resources/styles/templates, VisualStates. Source of truth for syntax: the public XAML language spec (MS-XAML and "XAML overview" docs) which describes the *language*, plus the public control API reference for names/properties. We do not implement MS's parser internals.

### 2.2 Dialect and names
- Extension `.laml` (plain XML). Compat mode also loads `.xaml` text (not `.xbf`, see 5).
- Namespace URI: accept the public presentation URI `http://schemas.microsoft.com/winfx/2006/xaml/presentation` and `.../xaml` (required for compat) **and** define our own `http://schemas.leeway.dev/laml/2026` (final domain: owner decision). Same element names (`Button`, `StackPanel`...) in both: a LAML file using our URI maps to our toolkit, a `.xaml` using Microsoft's URI maps to the `Windows.UI.Xaml` class names.
- Class ids for LAML-native extras (e.g. a hypothetical `Leeway.Laml.Controls.Card`) live under `Leeway.*`; they are *additive*, never replace `Windows.UI.Xaml.*`.

### 2.3 Pipeline
parser (XmlLite-class, ReactOS has `xmllite`) -> element tree via the type system (`.winmd` + built-in control table) -> dependency-property store -> style/resource resolution -> measure/arrange layout -> render tree -> backend.

### 2.4 Layout
Measure/Arrange as documented (public): Grid, StackPanel, Canvas, ScrollViewer, Border, WrapPanel; HorizontalAlignment/VerticalAlignment/Margin/Width/Height/MinMax. Deterministic, DPI-aware (per-monitor DPI via `GetDpiForWindow`).

### 2.5 Rendering backend
Phase 1: **GDI+ and DirectWrite** (both in tree: `gdiplus`, `dwrite`, `windowscodecs`) into a layered/child HWND, double-buffered, text via DirectWrite where available else GDI. Phase 2: **Direct2D** (`d2d1` is **absent from the tree**; Wine has a d2d1 we could import, license LGPL, big). Phase 3: composition via `dwmcore`/DWM visual layer, only when the WDDM/DX path exists (see WDDM docs). Rationale: GDI/GDI+ works on the BGA/VGA-class display drivers LiberNT runs on today, so demos run on the dx2400 and VMs without GPU.

### 2.6 Input and accessibility
Pointer/keyboard via standard window messages mapped to `PointerPressed` etc.; UI Automation via `uiautomationcore` (present). Text input via `windows.ui.core.textinput` (present).

## 3. Branding vs compatibility, explicit table

| Layer | Public/compat name (never changes) | Our branded name |
|---|---|---|
| Runtime DLLs | `combase.dll`, `wintypes.dll`, `Windows.*.dll`, `api-ms-win-core-winrt-*` | file `VersionInfo` ProductName "Leeway Runtime (LevRT)"; source dirs `levrt/` |
| Namespaces / class ids | `Windows.Foundation.*`, `Windows.UI.Xaml.*`, `Windows.Storage.*` | extras under `Leeway.*` only |
| Registry | `...\WindowsRuntime\ActivatableClassId` | `...\Leeway\LevRT\` for private config |
| Markup | `.xaml` text, MS presentation namespace URI | `.laml`, LAML namespace URI |
| Docs / tools | n/a | LevRT/LAML docs, `lamlc` (compiler/validator), `levrt-activate` debug tool |
| Debug/diagnostics | `RoOriginateError` strings | trace channel `LEVRT` |
| Version reporting | `Windows.System.Profile` as real Windows | `LevRTVersion` extra in our own key |

Rule of thumb: **if a third-party binary could mention the name, it is ABI and stays Microsoft's; if only our own tools/docs mention it, it is branded.** Fingerprinting/impersonation is not the goal: the product identity is shown in Properties > Details, About boxes, docs.

## 4. Milestones, demos, effort (engineer-months, one engineer, rough ±50%)

| M | Scope | Demo app | Effort |
|---|---|---|---|
| M0 | Test harness (WinRT conformance tests from public docs, WinRT `.idl` build), measure what the existing Wine layer passes; document activation behaviour | `levrt-activate Windows.Foundation.PropertyValue` prints the object's IIDs | 1 |
| M1 | Real `RoActivateInstance`/factory lookup (registry + built-in table + `.winmd` resolve), `RoGetMetaDataFile`, error origination, full HSTRING API, `Windows.Foundation` core | C++/WinRT hello (`winrt::init_apartment`, `PropertySet`, `IVector`) runs | 2 to 3 |
| M2 | Surface actually used by real inbox-style binaries: `Windows.Storage`, `Windows.Data.Json`, `Windows.UI.Core.CoreWindow`, `Windows.System`, `Windows.ApplicationModel` basics | A console WinRT sample reading/writing a file with `StorageFile`, JSON roundtrip | 2 to 3 |
| M3 | LAML parser (`.laml` + `.xaml` text), type system, dependency properties, resources/styles, object tree headless | `lamlc validate settings.laml` plus a unit test that loads a 100-element tree | 2 |
| M4 | Layout (Stack/Grid/Canvas/ScrollViewer/Border), controls (TextBlock, Button, TextBox, CheckBox, ToggleSwitch, ListView-lite), GDI+/DirectWrite renderer, input, DPI | **"Hello LAML"**: window with button and textbox that edits text, runs on dx2400/VM | 4 to 5 |
| M5 | Binding (OneWay/TwoWay), `ControlTemplate`, `VisualStateManager`, animations (basic), `Page`/navigation, theme resources light/dark | **LAML Settings page** (re-expression of our `systemsettings` pages in LAML) | 3 to 4 |
| M6 (stretch) | AppX/UWP activation (`CoreApplication`, `Windows.UI.Xaml.Application.Start`), `Windows.UI.Xaml.dll` ABI parity for common controls, d2d1 backend, `.xbf` loader | A trivial third-party UWP-style hello XAML app | open (6+) |

Total M0..M5 ≈ 14 to 20 engineer-months; M6 depends on how much real-world XAML apps must run.

## 5. Constraints and non-goals

- **No `.xbf` initially.** The binary XAML format is undocumented; loading it needs observation of Microsoft files, which conflicts with clean-room. Apps ship XAML compiled to `.xbf` inside `.pri`/`.pak`, so *most real UWP apps won't run through M5*; plain-text `.xaml` and LAML do. Revisit as an owner decision on legal basis.
- **No Microsoft `.winmd` / UI resources / fonts / icons** copied. All metadata authored from public docs.
- WinUI 3 (Windows App SDK) and Fluent visuals: out of scope; a LAML theme "Aureline" (see `parity-audit/AURELINE-ASTERION.md`) is a natural fit for our look.
- No promise of pixel parity with Windows controls; target is *behavioural* parity for the supported set.

## 6. What we reuse

- **Wine:** `combase`, `wintypes`, `rometadata`, `windows.*` stubs, `windows.ui` (corewindow, uisettings), possibly `d2d1`, `dcomp` stubs. All LGPL-2.1+ per tree conventions, same as the rest of the fork.
- **ReactOS:** `gdiplus`, `dwrite`, `windowscodecs`, `uxtheme`, `uiautomationcore`, `xmllite`, user32/gdi32.
- **Our code:** `systemsettings` page content (reuse behaviours, not layout), Aureline icons/theme, API Set table for `api-ms-win-core-winrt-*`.

## 7. Test plan

1. **Conformance (M0+):** per-API tests from public docs: HSTRING edge cases (embedded NULs, fast-pass strings), apartment rules, activation failure HRESULTs, `IInspectable::GetRuntimeClassName/GetTrustLevel`. Place under `modules/rostests/apitests/combase` and `winetests` where Wine tests exist (run Wine's `combase`/`wintypes` tests first as baseline).
2. **Golden-file tests (M3+):** `.laml` -> serialized element tree; layout tests assert measured/arranged rectangles; no screenshots.
3. **Rendering tests (M4+):** render to bitmap, compare with tolerance; run in VM, separate result from compile as always.
4. **Compatibility tests:** a small corpus of *our own authored* C++/WinRT programs built with the public C++/WinRT headers; later any third-party samples the owner supplies.
5. **API Set coverage:** `levrt` activation must run through our API Set resolver (`api-ms-win-core-winrt-*`); test with `t1`-style recognition.
6. **Reporting:** compile and test results reported separately; coverage claims only for validated paths.

## 8. Open questions for the owner

1. Namespace/domain for the LAML URI and product trademark ("LevRT", "LAML").
2. Is `.xbf` loading acceptable (needs a legal view)?
3. Priority of **third-party UWP/WinUI compatibility** vs **our own LAML apps first** (this design assumes the latter, then growth toward compat).
4. Direct2D: import Wine d2d1 (large, LGPL) or stay GDI+/DirectWrite until a GPU path exists?
5. Who owns WinRT conformance tests (suggest the same agent that owns API Sets).
