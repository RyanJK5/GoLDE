add_executable(GOLTest Testing.cpp)

target_link_libraries(GOLTest PRIVATE
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
    GTest::gtest_main
)


# Windows system libs
if(WIN32)
    target_link_libraries(GOLTest PRIVATE
        User32.lib 
        Gdi32.lib 
        Shell32.lib 
        ole32.lib
    )
elseif(UNIX AND NOT APPLE)
    # Linux: Link required system libraries
    target_link_libraries(GOLTest PRIVATE
        ${CMAKE_DL_LIBS}  # for dlopen, dlsym, etc.
        pthread           # for threading
    )
endif()