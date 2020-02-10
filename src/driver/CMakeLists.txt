add_library(driver OBJECT spi.cpp sx.cpp sx126x.cpp)

target_include_directories(driver
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )


target_link_libraries(driver PUBLIC chibios)   