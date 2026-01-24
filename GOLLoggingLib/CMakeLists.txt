file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

add_library(GOLLoggingLib STATIC ${SOURCES} ${HEADERS})

target_include_directories(GOLLoggingLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# GOLLoggingLib uses GL/glew.h for GL_DEBUG macros
target_link_libraries(GOLLoggingLib
    PUBLIC
        glew
)

# Add /FS flag for MSVC to avoid PDB file access issues during parallel builds
if(MSVC)
    target_compile_options(GOLLoggingLib PRIVATE /FS)
endif()