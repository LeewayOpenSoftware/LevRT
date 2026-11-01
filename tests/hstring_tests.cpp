// M0 HSTRING conformance tests, written from the public WinRT HSTRING documentation.
// Tiny runner on purpose: no dependency, same binary runs on Windows (baseline) and LiberNT.
#include <windows.h>
#include <winstring.h>
#include <cstdio>
#include <cwchar>

static int g_fail, g_run;
#define CHECK(c) do { ++g_run; if (!(c)) { ++g_fail; std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

static void test_basic()
{
    HSTRING h = nullptr;
    CHECK(WindowsCreateString(L"abc", 3, &h) == S_OK);
    CHECK(WindowsGetStringLen(h) == 3);
    UINT32 n = 0;
    const WCHAR* b = WindowsGetStringRawBuffer(h, &n);
    CHECK(n == 3 && b && !wcscmp(b, L"abc") && b[3] == 0);   // always NUL-terminated
    CHECK(WindowsIsStringEmpty(h) == FALSE);
    CHECK(WindowsDeleteString(h) == S_OK);
}
static void test_empty()
{
    HSTRING h = (HSTRING)1;
    CHECK(WindowsCreateString(nullptr, 0, &h) == S_OK);
    CHECK(h == nullptr);                                      // empty string is a NULL handle
    CHECK(WindowsGetStringLen(nullptr) == 0);
    CHECK(WindowsIsStringEmpty(nullptr) == TRUE);
    UINT32 n = 99;
    const WCHAR* b = WindowsGetStringRawBuffer(nullptr, &n);
    CHECK(n == 0 && b && b[0] == 0);
    CHECK(WindowsDeleteString(nullptr) == S_OK);
    CHECK(WindowsCreateString(L"x", 0, &h) == S_OK && h == nullptr);
}
static void test_invalid()
{
    HSTRING h = nullptr;
    CHECK(WindowsCreateString(nullptr, 1, &h) == E_POINTER);  // NULL source with length > 0
    CHECK(WindowsCreateString(L"a", 1, nullptr) == E_INVALIDARG || WindowsCreateString(L"a", 1, nullptr) == E_POINTER);
}
static void test_embedded_nul()
{
    HSTRING h = nullptr;
    CHECK(WindowsCreateString(L"a\0b", 3, &h) == S_OK);
    UINT32 n = 0;
    const WCHAR* b = WindowsGetStringRawBuffer(h, &n);
    CHECK(n == 3 && b[0] == L'a' && b[1] == 0 && b[2] == L'b');
    WindowsDeleteString(h);
}
static void test_dup_compare_concat()
{
    HSTRING a = nullptr, b = nullptr, d = nullptr, c = nullptr;
    WindowsCreateString(L"foo", 3, &a);
    WindowsCreateString(L"bar", 3, &b);
    CHECK(WindowsDuplicateString(a, &d) == S_OK);
    INT32 r = 7;
    CHECK(WindowsCompareStringOrdinal(a, d, &r) == S_OK && r == 0);
    CHECK(WindowsCompareStringOrdinal(a, b, &r) == S_OK && r > 0);
    CHECK(WindowsCompareStringOrdinal(nullptr, a, &r) == S_OK && r < 0);
    CHECK(WindowsConcatString(a, b, &c) == S_OK && WindowsGetStringLen(c) == 6 && !wcscmp(WindowsGetStringRawBuffer(c, nullptr), L"foobar"));
    HSTRING e = nullptr;
    CHECK(WindowsConcatString(nullptr, nullptr, &e) == S_OK && e == nullptr);
    CHECK(WindowsConcatString(a, nullptr, &e) == S_OK && !wcscmp(WindowsGetStringRawBuffer(e, nullptr), L"foo"));
    WindowsDeleteString(e); WindowsDeleteString(c); WindowsDeleteString(d); WindowsDeleteString(b); WindowsDeleteString(a);
}
static void test_fast_pass()
{
    HSTRING_HEADER hdr; HSTRING h = nullptr;
    static const WCHAR s[] = L"fastpass";
    CHECK(WindowsCreateStringReference(s, 8, &hdr, &h) == S_OK);
    CHECK(WindowsGetStringLen(h) == 8);
    HSTRING dup = nullptr;
    CHECK(WindowsDuplicateString(h, &dup) == S_OK);           // dup of a reference must be a real copy
    CHECK(WindowsGetStringRawBuffer(dup, nullptr) != s);
    WindowsDeleteString(dup);
    // Windows 11 returns S_OK for a source not NUL-terminated at length (observed baseline, docs call it a precondition): not asserted.
}
static void test_substring()
{
    HSTRING a = nullptr, sub = nullptr;
    WindowsCreateString(L"abcdef", 6, &a);
    CHECK(WindowsSubstring(a, 2, &sub) == S_OK && !wcscmp(WindowsGetStringRawBuffer(sub, nullptr), L"cdef"));
    WindowsDeleteString(sub);
    CHECK(WindowsSubstringWithSpecifiedLength(a, 1, 3, &sub) == S_OK && !wcscmp(WindowsGetStringRawBuffer(sub, nullptr), L"bcd"));
    WindowsDeleteString(sub);
    CHECK(WindowsSubstring(a, 7, &sub) == E_BOUNDS);
    WindowsDeleteString(a);
}

int main()
{
    test_basic(); test_empty(); test_invalid(); test_embedded_nul();
    test_dup_compare_concat(); test_fast_pass(); test_substring();
    std::printf("hstring_tests: %d checks, %d failed\n", g_run, g_fail);
    return g_fail ? 1 : 0;
}
