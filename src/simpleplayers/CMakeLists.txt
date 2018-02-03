file(GLOB simpleplyers_SRC *.cpp *.hpp)
add_library(simpleplayers STATIC ${simpleplyers_SRC})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${top_srcdir}/src
        ${top_srcdir}/src/hex
        ${top_srcdir}/src/util
        ${top_srcdir}/src/book
        ${top_srcdir}/src/solver
        ${top_srcdir}/src/commonengine
        ${top_srcdir}/src/smartgame
        ${top_srcdir}/src/gtpengine)
target_link_libraries(simpleplayers hex book
       fuego_smartgame
       fuego_gtpengine
       util solver commonengine db)
