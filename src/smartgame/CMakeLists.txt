file(GLOB smartgame_SRC *.cpp *.h *.hpp)

add_library(fuego_smartgame STATIC ${smartgame_SRC})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(
        ${top_srcdir}/src
        ${top_srcdir}/src/gtpengine/)

