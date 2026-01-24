file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

message(STATUS "GOLAlgoLib sources found: ${SOURCES}")
message(STATUS "GOLAlgoLib headers found: ${HEADERS}")

if(NOT SOURCES)
    message(FATAL_ERROR "No source files found for GOLAlgoLib in ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

add_library(GOLAlgoLib STATIC ${SOURCES} ${HEADERS})

target_include_directories(GOLAlgoLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(GOLAlgoLib
    PUBLIC
        unordered_dense
        GOLLoggingLib
        GOLGraphicsLib
)

# Add /FS flag for MSVC to avoid PDB file access issues during parallel builds
if(MSVC)
    target_compile_options(GOLAlgoLib PRIVATE /FS)
endif()

message(STATUS "GOLAlgoLib configured successfully")