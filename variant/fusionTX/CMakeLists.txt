

set(tools "D:/bin/GNU Tools ARM Embedded/9 2019-q4-major")
set(CMAKE_C_COMPILER ${tools}/bin/arm-none-eabi-gcc.exe)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-none-eabi-gcc.exe)

add_compile_options(-mcpu=cortex-m4 -mthumb -O3 -ggdb -fomit-frame-pointer 
-falign-functions=16 -ffunction-sections -fdata-sections -fno-common  
-mfloat-abi=hard -mfpu=fpv4-sp-d16 -flto -Wall -Wextra -Wundef  
-DCORTEX_USE_FPU=TRUE )

add_link_options(-mcpu=cortex-m4 -mthumb -O3 -ggdb -fomit-frame-pointer -falign-functions=16 
-ffunction-sections -fdata-sections -fno-common -flto -mfloat-abi=hard -mfpu=fpv4-sp-d16 -nostartfiles)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

cmake_minimum_required(VERSION 3.16)

project(fusionTX VERSION 1.0 LANGUAGES C CXX ASM)

# need todo this a better way.
include_directories(cfg)

# add the components that we need
add_subdirectory(../../lib/lvgl lvgl)

    
add_executable(fusionTX
    main.cpp
)

add_executable(fusionSimple
    main-simple.cpp
)

target_link_libraries(fusionTX PUBLIC lvgl)
target_link_libraries(fusionSimple PUBLIC lvgl)
