# Base path for test files
set(STORM_TESTS_BASE_PATH "${PROJECT_SOURCE_DIR}/src/test/storm-compose")

# Test Sources
file(GLOB_RECURSE ALL_FILES ${STORM_TESTS_BASE_PATH}/*.h ${STORM_TESTS_BASE_PATH}/*.cpp)

register_source_groups_from_filestructure("${ALL_FILES}" test)

# Note that the tests also need the source files, except for the main file
include_directories(${GTEST_INCLUDE_DIR})

foreach (testsuite modelchecking)

	  file(GLOB_RECURSE TEST_${testsuite}_FILES ${STORM_TESTS_BASE_PATH}/${testsuite}/*.h ${STORM_TESTS_BASE_PATH}/${testsuite}/*.cpp)
      add_executable (test-compose-${testsuite} ${TEST_${testsuite}_FILES} ${STORM_TESTS_BASE_PATH}/storm-test.cpp)
	  target_link_libraries(test-compose-${testsuite} storm-compose storm-parsers)
	  target_link_libraries(test-compose-${testsuite} ${STORM_TEST_LINK_LIBRARIES})

	  add_dependencies(test-compose-${testsuite} test-resources)
	  add_test(NAME run-test-compose-${testsuite} COMMAND $<TARGET_FILE:test-compose-${testsuite}>)
      add_dependencies(tests test-compose-${testsuite})
	
endforeach ()
