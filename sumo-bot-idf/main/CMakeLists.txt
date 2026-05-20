idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES motor_controll line_detector ir_array ir_to_drive jsumo_module)