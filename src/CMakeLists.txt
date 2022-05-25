set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.74.0 COMPONENTS program_options) 

include_directories(./tests ./buffers)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    add_executable(robots-client robots-client.cpp) 
    target_link_libraries(robots-client ${Boost_LIBRARIES})
endif()
