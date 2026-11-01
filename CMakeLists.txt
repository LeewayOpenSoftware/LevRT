cmake_minimum_required(VERSION 3.20)
project(LevRT CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# xlang meta_reader (MIT, imported verbatim, see PROVENANCE.md). Needs <algorithm> force-included
# (missing include upstream) and the Windows 10 API level for CreateFile2.
add_library(levrt_meta_reader INTERFACE)
target_include_directories(levrt_meta_reader INTERFACE third_party/xlang/library)
target_compile_definitions(levrt_meta_reader INTERFACE _WIN32_WINNT=0x0A00 WINVER=0x0A00)
target_compile_options(levrt_meta_reader INTERFACE -include algorithm)

add_executable(meta_reader_smoke tests/meta_reader_smoke.cpp)
target_link_libraries(meta_reader_smoke PRIVATE levrt_meta_reader)

add_executable(hstring_tests tests/hstring_tests.cpp)
target_compile_definitions(hstring_tests PRIVATE _WIN32_WINNT=0x0A00)
target_link_libraries(hstring_tests PRIVATE runtimeobject)

add_executable(levrt-activate tools/levrt-activate/levrt-activate.cpp)
target_compile_definitions(levrt-activate PRIVATE _WIN32_WINNT=0x0A00 UNICODE _UNICODE)
target_link_options(levrt-activate PRIVATE -municode)
target_link_libraries(levrt-activate PRIVATE runtimeobject ole32)
