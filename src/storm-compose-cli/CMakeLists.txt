# Create storm-compose.

file(GLOB_RECURSE STORM_COMPOSE_CLI_SOURCES ${PROJECT_SOURCE_DIR}/src/storm-compose-cli/*/*.cpp)
add_executable(storm-compose-cli ${PROJECT_SOURCE_DIR}/src/storm-compose-cli/storm-compose.cpp ${STORM_COMPOSE_CLI_SOURCES})
target_link_libraries(storm-compose-cli storm-compose storm-cli-utilities) # Adding headers for xcode
set_target_properties(storm-compose-cli PROPERTIES OUTPUT_NAME "storm-compose")

add_dependencies(binaries storm-compose-cli)

# installation
install(TARGETS storm-compose-cli RUNTIME DESTINATION bin LIBRARY DESTINATION lib OPTIONAL)
