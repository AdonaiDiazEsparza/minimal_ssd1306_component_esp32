file(GLOB_RECURSE CLI_CORE_SRC
    "src/*.c"
    "*.c"
)

idf_component_register(
    SRCS ${CLI_CORE_SRC}
    INCLUDE_DIRS
        "include"
        "include/fonts"
    REQUIRES driver freertos
)
