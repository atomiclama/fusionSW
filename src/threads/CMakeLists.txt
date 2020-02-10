
add_library(thread OBJECT convThread.cpp inThread.c modemThread.cpp outThread.cpp serialThread.cpp)

target_include_directories(thread
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

target_link_libraries(thread PUBLIC proto driver util chibios)     