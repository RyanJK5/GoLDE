file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "include/*.h")

message(STATUS "GOLCoreLib sources found: ${SOURCES}")

add_library(GOLCoreLib STATIC ${SOURCES} ${HEADERS})

target_include_directories(GOLCoreLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(GOLCoreLib
    PUBLIC
        GOLAlgoLib
        GOLGraphicsLib
        GOLGuiLib
        GOLLoggingLib
        glew
        glfw
)

# Add /FS flag for MSVC to avoid PDB file access issues during parallel builds
if(MSVC)
    target_compile_options(GOLCoreLib PRIVATE /FS)
endif()

message(STATUS "GOLCoreLib (core library) configured successfully")