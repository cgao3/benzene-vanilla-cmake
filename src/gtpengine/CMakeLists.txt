file(GLOB gtpengine_SRC *.cpp *.h *.hpp)

include_directories(${top_srcdir}/src)
include_directories(${Boost_INCLUDE_DIRS})

add_library(fuego_gtpengine STATIC ${gtpengine_SRC})

