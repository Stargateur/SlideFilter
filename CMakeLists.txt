project(slide-filter)

set(slide-filter_SOURCES
      src/lib.c)

add_library(slide-filter MODULE
      ${slide-filter_SOURCES} EXCLUDE_FROM_ALL)

add_subdirectory(obs-studio build)

target_link_libraries(slide-filter
      libobs)
