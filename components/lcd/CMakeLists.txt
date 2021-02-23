file(GLOB_RECURSE SOURCES config/*.c device/*.c fonts/*.c images/*.c task/*.c)

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS config device fonts image task)