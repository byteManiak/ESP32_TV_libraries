FILE(GLOB_RECURSE app_sources (*.*))

set(include_dirs "." "io" "memory" "stats" "ux")
idf_component_register(SRCS ${app_sources}
		               INCLUDE_DIRS ${include_dirs})
