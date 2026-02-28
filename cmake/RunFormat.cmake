# 1. Find all files
file(GLOB_RECURSE FILES 
    "src/*.cpp" "src/*.h" 
    "include/*.h" "tests/*.cpp"
)

# 2. Filter out what you don't want (Regex)
list(FILTER FILES EXCLUDE REGEX "Dependencies/|build/|out/")

# 3. Run clang-format on each file (prevents argument limit issues)
foreach(FILE ${FILES})
    execute_process(COMMAND clang-format -i ${FILE})
endforeach()