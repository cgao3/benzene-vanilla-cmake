file(GLOB book_SRC *.cpp *.hpp)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${top_srcdir}/src
        ${top_srcdir}/src/util
        ${top_srcdir}/src/hex
        ${top_srcdir}/src/smartgame
        ${top_srcdir}/src/gtpengine)
add_library(book STATIC ${book_SRC})

target_link_libraries(book 
        util
        hex
        fuego_smartgame
        fuego_gtpengine
        pthread
        db)

