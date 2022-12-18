file(GLOB smartgame_SRC *.cpp *.h *.hpp)

if(${APPLE})
  # otherwise ranlib will fail with the "has no symbols" error
  list(FILTER smartgame_SRC EXCLUDE REGEX ".*SgProcess\..*")
endif()

add_library(fuego_smartgame STATIC ${smartgame_SRC})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(
        ${top_srcdir}/src
        ${top_srcdir}/src/gtpengine/)

