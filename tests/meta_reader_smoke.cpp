// Smoke test for the imported xlang meta_reader: opens a .winmd given on the command line (test input is never committed).
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include "meta_reader.h"
using namespace xlang::meta::reader;
int main(int argc, char** argv)
{
    if (argc < 2) { std::puts("usage: meta_reader_smoke <file.winmd>"); return 0; }
    cache c{ std::vector<std::string>{ argv[1] } };
    size_t types = 0, ifaces = 0, classes = 0, methods = 0;
    for (auto&& ns : c.namespaces())
        for (auto&& t : ns.second.types)
        {
            ++types;
            auto const& td = t.second;
            if (get_category(td) == category::interface_type) ++ifaces;
            if (get_category(td) == category::class_type) ++classes;
            methods += size(td.MethodList());
        }
    std::printf("namespaces=%zu types=%zu interfaces=%zu classes=%zu methods=%zu\n", c.namespaces().size(), types, ifaces, classes, methods);
    return 0;
}
