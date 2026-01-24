file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

message(STATUS "GOLGraphicsLib sources: ${SOURCES}")

add_library(GOLGraphicsLib STATIC ${SOURCES} ${HEADERS})

target_include_directories(GOLGraphicsLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(GOLGraphicsLib
    PUBLIC
        glm
        glew
        glfw
        imgui_lib
        OpenGL::GL
        GOLLoggingLib
)

# GLEW needs GLEW_STATIC defined
target_compile_definitions(GOLGraphicsLib PUBLIC GLEW_STATIC)

# Add /FS flag for MSVC to avoid PDB file access issues during parallel builds
if(MSVC)
    target_compile_options(GOLGraphicsLib PRIVATE /FS)
endif()