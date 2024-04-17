if( (NOT ${GTest_FOUND}) AND MAKE_TEST_EXE )
    update_submodule("${CMAKE_CURRENT_LIST_DIR}/gtest")
    add_subdirectory(gtest)
endif()
