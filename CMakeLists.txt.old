
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

project(fusionSW VERSION 1.0 LANGUAGES C CXX ASM)

# specify the C++ standard
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(variant/atomRx variant/atomRx/cfg)

include_directories(
        src/ext/chibios/os/common/portability/GCC
        src/ext/chibios/os/common/startup/ARMCMx/compilers/GCC
        src/ext/chibios/os/common/startup/ARMCMx/devices/STM32F4xx
        src/ext/chibios/os/common/ext/ARM/CMSIS/Core/Include
        src/ext/chibios/os/common/ext/ST/STM32F4xx
        src/ext/chibios/os/hal/ports/common/ARMCMx
        src/ext/chibios/os/hal/ports/STM32/STM32F4xx
        src/ext/chibios/os/hal/ports/STM32/LLD/ADCv2 
        src/ext/chibios/os/hal/ports/STM32/LLD/CANv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/CRYPv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/DACv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/DMAv2 
        src/ext/chibios/os/hal/ports/STM32/LLD/EXTIv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/GPIOv2 
        src/ext/chibios/os/hal/ports/STM32/LLD/I2Cv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/MACv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/OTGv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/QUADSPIv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/RTCv2 
        src/ext/chibios/os/hal/ports/STM32/LLD/SPIv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/SDIOv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/TIMv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/USARTv1 
        src/ext/chibios/os/hal/ports/STM32/LLD/xWDGv1 
        src/ext/chibios/os/hal/osal/rt-nil 
        src/ext/chibios/os/oslib/include 
        src/ext/chibios/os/common/ports/ARMCMx 
        src/ext/chibios/os/common/ports/ARMCMx/compilers/GCC 
        src/ext/chibios/os/hal/lib/streams
        
        src/ext/chibios/os/rt/include
        src/ext/chibios/os/hal/include
        src/ext/chibios/os/license
        )



add_subdirectory(src/driver)
add_subdirectory(src/protocol)
add_subdirectory(src/threads)
add_subdirectory(src/utils)

add_executable(${PROJECT_NAME}
  variant/atomRx/main.cpp
  variant/atomRx/board.c
)

target_link_libraries(${PROJECT_NAME} PUBLIC proto util log thread driver chibios hal port lld)

#target_include_directories(${PROJECT_NAME} PUBLIC )

       
              
