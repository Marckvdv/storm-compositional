file(GLOB_RECURSE ALL_FILES ${PROJECT_SOURCE_DIR}/src/storm-compose/*.h ${PROJECT_SOURCE_DIR}/src/storm-compose/*.cpp)

register_source_groups_from_filestructure("${ALL_FILES}" storm-compose)



file(GLOB_RECURSE STORM_COMPOSE_SOURCES ${PROJECT_SOURCE_DIR}/src/storm-compose/*/*.cpp)
file(GLOB_RECURSE STORM_COMPOSE_HEADERS ${PROJECT_SOURCE_DIR}/src/storm-compose/*/*.h)


# Create storm-compose.
add_library(storm-compose SHARED ${STORM_COMPOSE_SOURCES} ${STORM_COMPOSE_HEADERS})

# Remove define symbol for shared libstorm.
set_target_properties(storm-compose PROPERTIES DEFINE_SYMBOL "")
#add_dependencies(storm resources)
list(APPEND STORM_TARGETS storm-compose)
set(STORM_TARGETS ${STORM_TARGETS} PARENT_SCOPE)

target_link_libraries(storm-compose PUBLIC storm storm-parsers ${STORM_COMPOSE_LINK_LIBRARIES})

# Install storm headers to include directory.
foreach(HEADER ${STORM_COMPOSE_HEADERS})
    string(REGEX REPLACE "${PROJECT_SOURCE_DIR}/src/?" "" RELATIVE_HEADER_PATH ${HEADER})
    string(REGEX MATCH "(.*)[/\\]" RELATIVE_DIRECTORY ${RELATIVE_HEADER_PATH})
    string(REGEX REPLACE "${RELATIVE_DIRECTORY}/?" "" HEADER_FILENAME ${RELATIVE_HEADER_PATH})
    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/include/${RELATIVE_DIRECTORY}${HEADER_FILENAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/include/${RELATIVE_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E copy ${HEADER} ${CMAKE_BINARY_DIR}/include/${RELATIVE_DIRECTORY}${HEADER_FILENAME}
            DEPENDS ${HEADER}
    )
list(APPEND STORM_COMPOSE_OUTPUT_HEADERS "${CMAKE_BINARY_DIR}/include/${RELATIVE_DIRECTORY}${HEADER_FILENAME}")
endforeach()
add_custom_target(copy_storm_compose_headers DEPENDS ${STORM_COMPOSE_OUTPUT_HEADERS} ${STORM_COMPOSE_HEADERS})
add_dependencies(storm-compose copy_storm_compose_headers)

# installation
install(TARGETS storm-compose RUNTIME DESTINATION bin LIBRARY DESTINATION lib OPTIONAL)

