# 1. Find all files
file(GLOB_RECURSE FILES 
    "${PROJECT_SOURCE_DIR}/GOL*/src/*.cpp" 
    "${PROJECT_SOURCE_DIR}/GOL*/include/*.h" 
)

# 2. Filter out what you don't want (Regex)
list(FILTER FILES EXCLUDE REGEX "Dependencies/|build/|out/")

if(FILES)
    message("A ${PROJECT_SOURCE_DIR}")
else()
    message("B ${PROJECT_SOURCE_DIR}")
endif()

# 3. Run clang-format on each file (prevents argument limit issues)
foreach(FILE ${FILES})
    message (STATUS "Formatting: ${FILE}") 
    execute_process(COMMAND clang-format -style=file -i ${FILE})
endforeach()