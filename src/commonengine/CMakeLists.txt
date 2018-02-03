file(GLOB commonengine_SRC *.cpp *.hpp)
add_library(commonengine STATIC ${commonengine_SRC})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${top_srcdir}/src
        ${top_srcdir}/src/hex
        ${top_srcdir}/src/util
        ${top_srcdir}/src/book
        ${top_srcdir}/src/solver
        ${top_srcdir}/src/smartgame
        ${top_srcdir}/src/gtpengine)
target_link_libraries(commonengine
        hex 
        util 
        book 
        fuego_smartgame
        fuego_gtpengine
        solver
        db)
