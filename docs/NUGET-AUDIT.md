# WinUI 3 build dependency audit (microsoft/microsoft-ui-xaml @ 8da148dcd8ed65ea8b8ae664fbe45c9cc82e372b)

Source: `Packages.props`, `packages.x64.config`, `NuGet.config`, `Directory.Build.props` (read from the sparse clone). Only names/versions come from the repo; license column is **verified = read in this session**, **(k)** = from memory, not verified. No package was downloaded.

| Package | Role | License | LevRT status |
|---|---|---|---|
| Microsoft.Windows.SDK.cpp.x64 10.0.26100.x | Windows SDK headers/libs (`packages.x64.config`) | Windows SDK terms (proprietary redistribution limits) (k) | **Do not bundle**; we use our own headers |
| Microsoft.FrameworkUdk | headers used by `xcp/core` (`FrameworkUdk/Theming.h`, `FrameworkInputView.h`, `MediaPlayerExtension.h`, `BackButtonIntegration.h`) | unknown; distributed from the `WinUI.Dependencies` Azure feed (verified in NuGet.config); not on the repo | **Blocker**: closed-source/unknown license; the 4 includes must be replaced by our own stubs, or those features cut |
| Microsoft.Internal.WinUIDetails | `Directory.Build.props` has `WinUIDetailsRepoLocation` empty | Microsoft-internal by name | **Blocker unless optional**; treat as proprietary, exclude |
| IXP transport package (`LiftedIXPTransportPackageName`) | Windows App SDK transport | unknown | exclude |
| Microsoft.WindowsAppSDK / .WinUI | Windows App SDK runtime (composition, input, windowing) | Microsoft software license for binaries (k) | exclude binaries; their IDL/winmd must be re-authored |
| Microsoft.Windows.SDK.BuildTools, .MSIX | midlrt/mt/signing tooling | Microsoft terms (k) | build-tool only, not redistributed; midlrt output (generated headers) is derived data, see SPIKE-B |
| Microsoft.Windows.CsWinRT | C# projection | MIT (verified for repo, OSS-SURVEY) | not needed |
| Microsoft.Web.WebView2 | WebView2 control | WebView2 SDK/runtime license (proprietary redistributable) (k) | exclude |
| Microsoft.Graphics.Win2D, Win2d.uwp | 2D rendering | MIT (k) | optional reference |
| CommunityToolkit.WinUI.* | extra controls (samples/tests) | MIT (k) | not needed |
| Newtonsoft.Json | JSON | MIT (k) | not needed |
| DiffPlex 1.2.1 | test diffing | Apache-2.0 (k) | tests only; **do not import** |
| ColorCode.Core | samples | MIT (k) | not needed |
| MSTest.*, Microsoft.NET.Test.Sdk, Microsoft.TestPlatform.TestHost, Microsoft.Taef | tests | MIT / Microsoft test framework (k) | not needed |
| MUXCustomBuildTasks, Microsoft.SourceLink.AzureRepos.Git | build tasks | MIT (k) / internal | not needed |
| wil (`wil\Result.h` included from `precomp.h`) | helper library, not a NuGet entry | MIT (microsoft/wil) (k) | could import as a separate row after its own check |
| `NOTICE.md` third-party | ankerl::unordered_dense MIT, Antlr4 BSD-3, GSL MIT, **Grammars-v4 EPL-1.0 (exclude)**, SafeInt MIT, TinyRGB CC0, WPF excerpts MIT | verified | licscan "flag" on NOTICE.md is just the aggregate file mixing BSD+MIT |

Summary: of the packages the *runtime* needs (FrameworkUdk, WinUIDetails, IXP transport, App SDK, Windows SDK) **none is confirmed MIT and at least two (FrameworkUdk, WinUIDetails) are closed/internal**. The 5,511 source files of the scanned subset do carry MIT headers (see `licscan-winui-subset-8da148d.csv`; 307 have none and rely on the repo LICENSE; 0 flagged except `NOTICE.md`).
