project(slide-filter)

set(slide-filter_SOURCES
      src/lib.c)

add_library(slide-filter MODULE
      ${slide-filter_SOURCES})

add_subdirectory(obs-studio build EXCLUDE_FROM_ALL)

target_link_libraries(slide-filter
      libobs)
