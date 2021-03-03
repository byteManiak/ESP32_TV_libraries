FILE(GLOB_RECURSE app_sources (*.*))

set(include_dirs "." "io" "memory" "net" "stats" "util" "ux" "vga" "widget")
idf_component_register(SRCS ${app_sources}
		               INCLUDE_DIRS ${include_dirs})

# Suppress compiler when the project is built in release mode
set_source_files_properties(${app_sources} PROPERTIES COMPILE_FLAGS "-Wno-unused-label")
