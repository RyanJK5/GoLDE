find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
    # Use a script to handle the heavy lifting. 
    # This avoids the "Argument list too long" error.
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} 
            -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR} 
            -P "${PROJECT_SOURCE_DIR}/cmake/RunFormat.cmake"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Formatting project files..."
    )
else()
    message(WARNING "clang-format not found. 'format' target will not be available.")
endif()