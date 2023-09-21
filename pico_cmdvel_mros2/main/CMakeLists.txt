idf_component_register(SRCS "pico_cmdvel_mros2.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES "pico_driver"
                    PRIV_REQUIRES "mros2-esp32")
