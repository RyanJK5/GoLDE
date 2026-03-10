add_executable(GOLExecutable Application.cpp)

target_link_libraries(GOLExecutable PRIVATE
    GOLCoreLib  
    GOLGuiLib
    GOLGraphicsLib
    GOLAlgoLib
    GOLLoggingLib
    imgui_lib
    glew
    glfw
    nfd
    OpenGL::GL
)

# Windows system libs
if(WIN32)
    target_link_libraries(GOLExecutable PRIVATE
        User32.lib 
        Gdi32.lib 
        Shell32.lib 
        ole32.lib
    )
elseif(UNIX AND NOT APPLE)
    # Linux: Link required system libraries
    target_link_libraries(GOLExecutable PRIVATE
        ${CMAKE_DL_LIBS}  # for dlopen, dlsym, etc.
        pthread           # for threading
    )
endif()

set_target_properties(GOLExecutable PROPERTIES
    WIN32_EXECUTABLE $<CONFIG:Release>
)
if(MSVC)
    # Allows using int main() even with WIN32_EXECUTABLE enabled
    set_target_properties(GOLExecutable PROPERTIES 
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

# Copy resources
add_custom_command(TARGET GOLExecutable POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:GOLExecutable>/resources
)

add_custom_command(TARGET GOLExecutable POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:GOLExecutable>/config
)

add_custom_command(TARGET GOLExecutable POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/templates
        $<TARGET_FILE_DIR:GOLExecutable>/templates
)