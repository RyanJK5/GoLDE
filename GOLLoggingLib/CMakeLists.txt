set(SOURCES
    src/Logging.cpp
)

set(HEADERS
    include/Logging.hpp
)

add_library(GOLLoggingLib STATIC ${SOURCES} ${HEADERS})

target_include_directories(GOLLoggingLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# GOLLoggingLib uses GL/glew.h for GL_DEBUG macros
target_link_libraries(GOLLoggingLib
    PRIVATE
        gol_compiler_options
    PUBLIC
        glew
)