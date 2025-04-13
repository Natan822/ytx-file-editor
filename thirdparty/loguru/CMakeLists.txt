include(FetchContent)

FetchContent_Declare(
    loguru
    GIT_REPOSITORY https://github.com/emilk/loguru.git
    GIT_TAG v2.1.0
)

FetchContent_MakeAvailable(loguru)

target_include_directories(
    YTX-File-Editor
    PUBLIC
    ${loguru_SOURCE_DIR}
)

target_sources(
    YTX-File-Editor
    PUBLIC
    ${loguru_SOURCE_DIR}/loguru.cpp 
)