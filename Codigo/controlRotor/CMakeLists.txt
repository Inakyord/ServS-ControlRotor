if (TARGET tinyusb_device)
    add_executable(controlRotor
            controlRotor.cpp
            funciones.cpp
            )

    #add_subdirectory(/Users/InakyO/Pico/pico-arduino-compat/libs/liquidcrystal build_pac_liquidcrystal)

    # pull in common dependencies
    target_link_libraries(controlRotor
	pico_stdlib
	hardware_gpio
	hardware_adc
	pac-liquidcrystal
	pico_multicore
	pico_bootsel_via_double_reset
	)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(controlRotor 1)
    pico_enable_stdio_uart(controlRotor 0)

    # activa el acceder al modo BOOTSEL reiniciando dos veces rapido
    target_compile_definitions(controlRotor PRIVATE
	PICO_BOOTSEL_VIA_DOUBLE_RESET_ACTIVITY_LED=15
	)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(controlRotor)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building controlRotor because TinyUSB submodule is not initialized in the SDK")
endif()
