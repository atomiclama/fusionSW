
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

project(fusion VERSION 1.0 LANGUAGES C CXX ASM)

# specify the C++ standard
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)


# need to do multiple cfg 
# then can do link targets from a particular build cfg
# so specify uP and a particular cfg file.



# Each add here will add a link variant based on the current config.
add_subdirectory(variant/fusionDisplay)
add_subdirectory(variant/fusionTX)
add_subdirectory(variant/atomRx)

# need todo this a better way.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/variant/fusionDisplay)

# add the components that we need
add_subdirectory(lib/lvgl)

add_subdirectory(src/driver)
add_subdirectory(src/protocol)
add_subdirectory(src/threads)
add_subdirectory(src/utils)




       
              
