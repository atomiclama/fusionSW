
add_library(chibios OBJECT 
            chibios/os/rt/src/chsys.c 
            chibios/os/rt/src/chdebug.c 
            chibios/os/rt/src/chtrace.c 
            chibios/os/rt/src/chvt.c 
            chibios/os/rt/src/chschd.c 
            chibios/os/rt/src/chthreads.c 
            chibios/os/rt/src/chtm.c 
            chibios/os/rt/src/chstats.c 
            chibios/os/rt/src/chregistry.c 
            chibios/os/rt/src/chsem.c 
            chibios/os/rt/src/chmtx.c 
            chibios/os/rt/src/chcond.c 
            chibios/os/rt/src/chevents.c 
            chibios/os/rt/src/chmsg.c 
            chibios/os/rt/src/chdynamic.c       


            chibios/os/oslib/src/chmboxes.c 
            chibios/os/oslib/src/chmemcore.c 
            chibios/os/oslib/src/chmemheaps.c 
            chibios/os/oslib/src/chmempools.c 
            chibios/os/oslib/src/chpipes.c 
            chibios/os/oslib/src/chobjcaches.c 
            chibios/os/oslib/src/chdelegates.c 
            chibios/os/oslib/src/chfactory.c     
)

add_library(hal OBJECT 
            chibios/os/hal/src/hal.c
            chibios/os/hal/src/hal_st.c 
            chibios/os/hal/src/hal_buffers.c 
            chibios/os/hal/src/hal_queues.c 
            chibios/os/hal/src/hal_flash.c 
            chibios/os/hal/src/hal_mmcsd.c 
            chibios/os/hal/src/hal_adc.c 
            chibios/os/hal/src/hal_can.c 
            chibios/os/hal/src/hal_crypto.c 
            chibios/os/hal/src/hal_dac.c 
            chibios/os/hal/src/hal_efl.c 
            chibios/os/hal/src/hal_gpt.c 
            chibios/os/hal/src/hal_i2c.c 
            chibios/os/hal/src/hal_i2s.c 
            chibios/os/hal/src/hal_icu.c 
            chibios/os/hal/src/hal_mac.c 
            chibios/os/hal/src/hal_mmc_spi.c 
            chibios/os/hal/src/hal_pal.c 
            chibios/os/hal/src/hal_pwm.c 
            chibios/os/hal/src/hal_rtc.c 
            chibios/os/hal/src/hal_sdc.c 
            chibios/os/hal/src/hal_serial.c 
            chibios/os/hal/src/hal_serial_usb.c 
            chibios/os/hal/src/hal_sio.c 
            chibios/os/hal/src/hal_spi.c 
            chibios/os/hal/src/hal_trng.c 
            chibios/os/hal/src/hal_uart.c 
            chibios/os/hal/src/hal_usb.c 
            chibios/os/hal/src/hal_wdg.c 
            chibios/os/hal/src/hal_wspi.c

            chibios/os/hal/osal/rt-nil/osal.c

            chibios/os/hal/lib/streams/chprintf.c 
            chibios/os/hal/lib/streams/memstreams.c 
            chibios/os/hal/lib/streams/nullstreams.c
         )

add_library(port OBJECT 
            chibios/os/hal/ports/common/ARMCMx/nvic.c 
            chibios/os/hal/ports/STM32/STM32F4xx/stm32_isr.c 
            chibios/os/hal/ports/STM32/STM32F4xx/hal_lld.c  
            chibios/os/hal/ports/STM32/STM32F4xx/hal_efl_lld.c

            chibios/os/common/startup/ARMCMx/compilers/GCC/crt1.c
            chibios/os/common/startup/ARMCMx/compilers/GCC/crt0_v7m.S 
            chibios/os/common/startup/ARMCMx/compilers/GCC/vectors.S
            chibios/os/common/ports/ARMCMx/chcore.c 
            chibios/os/common/ports/ARMCMx/chcore_v7m.c
            chibios/os/common/ports/ARMCMx/compilers/GCC/chcoreasm_v7m.S
)

add_library(lld OBJECT 
            chibios/os/hal/ports/STM32/LLD/ADCv2/hal_adc_lld.c
            chibios/os/hal/ports/STM32/LLD/CANv1/hal_can_lld.c
            chibios/os/hal/ports/STM32/LLD/CRYPv1/hal_crypto_lld.c
            chibios/os/hal/ports/STM32/LLD/DACv1/hal_dac_lld.c
            chibios/os/hal/ports/STM32/LLD/DMAv2/stm32_dma.c
            chibios/os/hal/ports/STM32/LLD/EXTIv1/stm32_exti.c
            chibios/os/hal/ports/STM32/LLD/GPIOv2/hal_pal_lld.c
            chibios/os/hal/ports/STM32/LLD/I2Cv1/hal_i2c_lld.c
            chibios/os/hal/ports/STM32/LLD/MACv1/hal_mac_lld.c
            chibios/os/hal/ports/STM32/LLD/OTGv1/hal_usb_lld.c
            chibios/os/hal/ports/STM32/LLD/QUADSPIv1/hal_wspi_lld.c
            chibios/os/hal/ports/STM32/LLD/RTCv2/hal_rtc_lld.c
            chibios/os/hal/ports/STM32/LLD/SPIv1/hal_i2s_lld.c
            chibios/os/hal/ports/STM32/LLD/SPIv1/hal_spi_lld.c
            chibios/os/hal/ports/STM32/LLD/SDIOv1/hal_sdc_lld.c
            chibios/os/hal/ports/STM32/LLD/TIMv1/
            chibios/os/hal/ports/STM32/LLD/USARTv1/hal_uart_lld.c
            chibios/os/hal/ports/STM32/LLD/xWDGv1/hal_wdg_lld.c
)

target_include_directories(hal
          PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/chibios/os/rt/include ${CMAKE_CURRENT_SOURCE_DIR}/chibios/os/hal/include
          )

target_link_libraries(chibios PUBLIC hal port lld )

target_include_directories(chibios
          PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/chibios/os/rt/include
          )
