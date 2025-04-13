set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(YTX-File-Editor Main.cpp UI.cpp Utils.cpp YtxFile.cpp App.cpp)

target_link_libraries(
    YTX-File-Editor
    PRIVATE
    SDL3::SDL3
    imgui
    nfd
)