file(GLOB util_SRC *.cpp *.hpp)
add_library(util STATIC ${util_SRC})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${top_srcdir}/src
        ${top_srcdir}/src/smartgame
        ${top_srcdir}/src/gtpengine)
target_link_libraries(util 
        hex
        fuego_smartgame
        fuego_gtpengine
        db)
