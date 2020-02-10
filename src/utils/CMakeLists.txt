add_library(util OBJECT crc.c msg_core.c)
add_library(log OBJECT log_core.c log_swo.c )

target_include_directories(util
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

target_include_directories(log
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

target_link_libraries(log PUBLIC chibios)     
target_link_libraries(util PUBLIC chibios)    