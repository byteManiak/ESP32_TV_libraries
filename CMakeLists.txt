FILE(GLOB_RECURSE app_sources (*.*))

set(include_dirs "." "io" "memory" "net" "stats" "util" "ux" "vga" "widget")
idf_component_register(SRCS ${app_sources}
		               INCLUDE_DIRS ${include_dirs}
                       REQUIRES esp_http_client esp_wifi nvs_flash audio_board audio_hal audio_sal audio_stream esp-adf-libs esp_peripherals bitluni)

# Suppress compiler when the project is built in release mode
set_source_files_properties(${app_sources} PROPERTIES COMPILE_FLAGS "-Wno-unused-label")
