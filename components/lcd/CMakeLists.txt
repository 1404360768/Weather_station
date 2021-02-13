set(srcs "config/*.c"
        "device/*.c"
        "fonts/*.c"
        "image/*.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS config device fonts image)