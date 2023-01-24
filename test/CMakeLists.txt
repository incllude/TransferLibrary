cmake_minimum_required(VERSION 3.22)
project(test)

set(CMAKE_CXX_STANDARD 23)

if(WIN32)
	set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}\\stage\\lib")
endif()
find_package(Boost COMPONENTS asio filesystem log log_setup)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions(${Boost_DEFINITIONS})

set(CMAKE_PREFIX_PATH "../install")
find_package(transfer CONFIG REQUIRED)

add_executable(rec rec.cpp)
add_executable(sen sen.cpp)

target_link_libraries(rec transfer::transfer)
target_link_libraries(rec ${Boost_LIBRARIES})
target_link_libraries(sen transfer::transfer)
target_link_libraries(sen ${Boost_LIBRARIES})
if(WIN32)
    target_link_libraries(sen ws2_32)
    target_link_libraries(rec ws2_32)
endif()
