add_library(utils moo_log.c moo_tools.c)

target_link_libraries(utils PRIVATE m)