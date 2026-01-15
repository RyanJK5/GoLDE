# OpenGL is now found in root CMakeLists.txt

# Header-only libraries
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)

add_library(unordered_dense INTERFACE)
target_include_directories(unordered_dense INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/unordered_dense)

add_library(fontawesome INTERFACE)
target_include_directories(fontawesome INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/FontAwesome)

# Add GoogleTest
add_subdirectory(GoogleTest)

message(STATUS "Header-only dependencies configured")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_PATH "x64")
else()
    set(ARCH_PATH "x86")
endif()

if(WIN32)
    set(PLAT_PATH "windows")
    set(GLFW_SYSTEM_LIBS "opengl32;ole32;gdi32;user32;kernel32;shell32")
    set(NFD_SYSTEM_LIBS "comctl32")
elseif(UNIX AND NOT APPLE)
    set(PLAT_PATH "linux")
    set(GLFW_SYSTEM_LIBS "libGL.so;libGLU.so;libX11.so")
    
    # Find GTK+3 using pkg-config for NFD
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    
    # Set NFD system libraries to include GTK3
    set(NFD_SYSTEM_LIBS ${GTK3_LIBRARIES})
    
    message(STATUS "GTK3 found: ${GTK3_LIBRARIES}")
    message(STATUS "GTK3 include dirs: ${GTK3_INCLUDE_DIRS}")
    message(STATUS "GTK3 library dirs: ${GTK3_LIBRARY_DIRS}")
elseif(APPLE)
    set(PLAT_PATH "macos")
endif()

set(GLEW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/Release/${PLAT_PATH}/${ARCH_PATH}/glew32s.lib")
add_library(glew STATIC IMPORTED GLOBAL)
    set_target_properties(glew PROPERTIES
        IMPORTED_LOCATION "${GLEW_LIB_PATH}"
        IMPORTED_LOCATION_DEBUG "${GLEW_LIB_PATH}"
        IMPORTED_LOCATION_RELEASE "${GLEW_LIB_PATH}"
        IMPORTED_LOCATION_MINSIZEREL "${GLEW_LIB_PATH}"
        IMPORTED_LOCATION_RELWITHDEBINFO "${GLEW_LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/include"
        INTERFACE_COMPILE_DEFINITIONS "GLEW_STATIC"
    )
    message(STATUS "GLEW configured: ${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/include")


set(GLFW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GLFW/lib/Release/${PLAT_PATH}/${ARCH_PATH}/glfw3.lib")
add_library(glfw3_lib STATIC IMPORTED GLOBAL)
    set_target_properties(glfw3_lib PROPERTIES
        IMPORTED_LOCATION "${GLFW_LIB_PATH}"
        IMPORTED_LOCATION_DEBUG "${GLFW_LIB_PATH}"
        IMPORTED_LOCATION_RELEASE "${GLFW_LIB_PATH}"
        IMPORTED_LOCATION_MINSIZEREL "${GLFW_LIB_PATH}"
        IMPORTED_LOCATION_RELWITHDEBINFO "${GLFW_LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/GLFW/include"
        INTERFACE_LINK_LIBRARIES "${GLFW_SYSTEM_LIBS}"
    )
    add_library(glfw ALIAS glfw3_lib)
    message(STATUS "GLFW configured: ${CMAKE_CURRENT_SOURCE_DIR}/GLFW/include")

set(NFD_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/NFD/lib/Release/${PLAT_PATH}/${ARCH_PATH}/nfd.lib")
add_library(nfd STATIC IMPORTED GLOBAL)
    set_target_properties(nfd PROPERTIES
        IMPORTED_LOCATION "${NFD_LIB_PATH}"
        IMPORTED_LOCATION_DEBUG "${NFD_LIB_PATH}"
        IMPORTED_LOCATION_RELEASE "${NFD_LIB_PATH}"
        IMPORTED_LOCATION_MINSIZEREL "${NFD_LIB_PATH}"
        IMPORTED_LOCATION_RELWITHDEBINFO "${NFD_LIB_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/NFD/include"
        INTERFACE_LINK_LIBRARIES "${NFD_SYSTEM_LIBS}"
    )
message(STATUS "NFD configured: ${CMAKE_CURRENT_SOURCE_DIR}/NFD/include")
    
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(GLEW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/linux/libGLEW.a")
    message(STATUS "GLEW: Using x64 library")
else()
    set(GLEW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/linux/x86/libGLEW.a")
    message(STATUS "GLEW: Using x86 library")
    endif()
    
add_subdirectory(ImGUI)