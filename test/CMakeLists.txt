add_executable(rec rec.cpp)
add_executable(sen sen.cpp)

target_link_libraries(rec transfer)
target_link_libraries(sen transfer)

if(WIN32)
    target_link_libraries(rec ws2_32)
    target_link_libraries(sen ws2_32)
endif()
