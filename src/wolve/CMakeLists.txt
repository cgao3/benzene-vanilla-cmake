file(GLOB wolve_SRC *.cpp *.hpp)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${top_srcdir}/src
        ${top_srcdir}/src/book
        ${top_srcdir}/src/hex
        ${top_srcdir}/src/util
        ${top_srcdir}/src/commonengine
        ${top_srcdir}/src/simpleplayers
        ${top_srcdir}/src/solver
        ${top_srcdir}/src/smartgame
        ${top_srcdir}/src/gtpengine)
link_directories(/usr/local/lib)
add_executable(wolve ${wolve_SRC})
target_link_libraries(wolve 
        hex 
        util 
        book 
        fuego_smartgame
        fuego_gtpengine
        solver 
        commonengine
        simpleplayers
        ${Boost_LIBRARIES}
        #boost_system boost_filesystem boost_program_options boost_thread boost_date_time
        db)
