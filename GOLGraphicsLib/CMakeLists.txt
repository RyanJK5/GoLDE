set(SOURCES
    src/Camera.cpp
    src/GraphicsHandler.cpp
    src/ShaderManager.cpp
)

set(HEADERS
    include/Camera.hpp
    include/GLBuffer.hpp
    include/GLException.hpp
    include/Graphics2D.hpp
    include/GraphicsHandler.hpp
    include/ShaderManager.hpp
)

add_library(GOLGraphicsLib STATIC ${SOURCES} ${HEADERS} )

target_include_directories(GOLGraphicsLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(GOLGraphicsLib
    PRIVATE
        gol_compiler_options
    PUBLIC
        glm
        glew
        glfw
        imgui_lib
        stb_image
        OpenGL::GL
        GOLLoggingLib
)

# GLEW needs GLEW_STATIC defined
target_compile_definitions(GOLGraphicsLib PUBLIC GLEW_STATIC)