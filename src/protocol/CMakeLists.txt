add_library(proto OBJECT crsf.c ibus.c)

target_link_libraries(proto PUBLIC util)     

target_include_directories(proto
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
