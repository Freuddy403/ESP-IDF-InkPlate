

# if(CONFIG_INKPLATE_VARIANT_10)
#     set(INKPLATE_VARIANT "INKPLATE_10")
# elseif(CONFIG_INKPLATE_VARIANT_6PLUS)
#     set(INKPLATE_VARIANT "INKPLATE_6PLUS")
# elseif(CONFIG_INKPLATE_VARIANT_6)
#     set(INKPLATE_VARIANT "INKPLATE_6")
# endif()

#FILE(GLOB_RECURSE inkplate_srcs src/**/*.cpp)

set(inkplate_srcs
    "src/services/wire.cpp"
    "src/drivers/mcp23017.cpp"
    "src/drivers/eink.cpp"
    "src/drivers/eink_6.cpp"
    "src/graphical/adafruit_gfx.cpp"
    "src/graphical/print.cpp"
    "src/graphical/graphics.cpp"
    "src/minimal/inkplate_minimal_test.cpp"
    "src/minimal/inkplate_platform_min.cpp"
    "src/graphical/image.cpp"
)


idf_component_register(
    SRCS ${inkplate_srcs}
    INCLUDE_DIRS 
    "include"
    "include/drivers"
    "include/tools"
    "include/services"
    "include/graphical"
    REQUIRES esp_timer esp_event esp_driver_gpio esp_driver_i2c driver
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC INKPLATE_6)
