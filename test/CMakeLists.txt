add_executable(test test.cpp)

target_link_libraries(test transfer)

if(WIN32)
    target_link_libraries(test ws2_32)
endif()
