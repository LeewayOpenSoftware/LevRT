// Spike A: does xlang meta_reader build with clang/mingw and read a .winmd?
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include "meta_reader.h"
using namespace xlang::meta::reader;
int main(int argc, char** argv)
{
    if (argc < 2) return 2;
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
