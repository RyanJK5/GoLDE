# OpenGL is now found in root CMakeLists.txt

# Header-only libraries
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)

add_library(unordered_dense INTERFACE)
target_include_directories(unordered_dense INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/unordered_dense)

add_library(fontawesome INTERFACE)
target_include_directories(fontawesome INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/FontAwesome)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb_image)

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
    set(NFD_SYSTEM_LIBS "comctl32")
elseif(UNIX AND NOT APPLE)
    set(PLAT_PATH "linux")
    
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

# Fetch GLFW from source
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
message(STATUS "GLFW configured from source")

# Fetch GLEW and build from the single source file
FetchContent_Declare(
    glew_fetch
    URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip
)
FetchContent_GetProperties(glew_fetch)
if(NOT glew_fetch_POPULATED)
    FetchContent_Populate(glew_fetch)
    add_library(glew STATIC ${glew_fetch_SOURCE_DIR}/src/glew.c)
    target_include_directories(glew SYSTEM PUBLIC ${glew_fetch_SOURCE_DIR}/include)
    target_compile_definitions(glew PUBLIC GLEW_STATIC)
    target_compile_options(glew PRIVATE -Wno-error)
    if(WIN32)
        target_link_libraries(glew PUBLIC opengl32)
    endif()
endif()
message(STATUS "GLEW configured from source")

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
    
add_subdirectory(ImGUI)