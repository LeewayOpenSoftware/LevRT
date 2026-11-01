// levrt-activate: activate a WinRT runtime class and print what the runtime reports.
// usage: levrt-activate <Fully.Qualified.RuntimeClass> [--factory]
// Uses only the public combase/WinRT ABI (roapi.h), so it runs against Windows or LiberNT.
#include <windows.h>
#include <roapi.h>
#include <winstring.h>
#include <inspectable.h>
#include <cstdio>
#include <cstdlib>

static void print_guid(const GUID& g)
{
    std::printf("  {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n", g.Data1, g.Data2, g.Data3,
                g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
}

int wmain(int argc, wchar_t** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: levrt-activate <RuntimeClass> [--factory]\n");
        return 2;
    }
    bool factory = argc > 2 && !wcscmp(argv[2], L"--factory");
    HRESULT hr = RoInitialize(RO_INIT_MULTITHREADED);
    std::printf("RoInitialize: 0x%08lX\n", (unsigned long)hr);
    if (FAILED(hr)) return 1;

    HSTRING cls = nullptr;
    hr = WindowsCreateString(argv[1], (UINT32)wcslen(argv[1]), &cls);
    std::printf("WindowsCreateString: 0x%08lX\n", (unsigned long)hr);
    if (FAILED(hr)) return 1;

    IInspectable* obj = nullptr;
    if (factory)
    {
        static const GUID iid_ia = {0x00000035, 0x0000, 0x0000, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
        hr = RoGetActivationFactory(cls, iid_ia, (void**)&obj);
        std::printf("RoGetActivationFactory(IActivationFactory): 0x%08lX\n", (unsigned long)hr);
    }
    else
    {
        hr = RoActivateInstance(cls, &obj);
        std::printf("RoActivateInstance: 0x%08lX\n", (unsigned long)hr);
    }
    int rc = FAILED(hr) ? 1 : 0;
    if (SUCCEEDED(hr) && obj)
    {
        HSTRING name = nullptr;
        if (SUCCEEDED(obj->GetRuntimeClassName(&name)) && name)
            std::printf("RuntimeClassName: %ls\n", WindowsGetStringRawBuffer(name, nullptr));
        WindowsDeleteString(name);
        TrustLevel tl = BaseTrust;
        if (SUCCEEDED(obj->GetTrustLevel(&tl))) std::printf("TrustLevel: %d\n", (int)tl);
        ULONG n = 0; IID* iids = nullptr;
        hr = obj->GetIids(&n, &iids);
        std::printf("GetIids: 0x%08lX count=%lu\n", (unsigned long)hr, n);
        if (SUCCEEDED(hr)) { for (ULONG i = 0; i < n; ++i) print_guid(iids[i]); CoTaskMemFree(iids); }
        obj->Release();
    }
    WindowsDeleteString(cls);
    RoUninitialize();
    return rc;
}
