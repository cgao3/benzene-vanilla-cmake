file(GLOB precomputation_SRC *.cpp *.hpp)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${top_srcdir}/src
        ${top_srcdir}/src/hex
	${top_srcdir}/src/util)
link_directories(/usr/local/lib)
add_executable(pattern_precomputation ${precomputation_SRC})
target_link_libraries(pattern_precomputation
        hex
        util
	${Boost_LIBRARIES}
	db)
