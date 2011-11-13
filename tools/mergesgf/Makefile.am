bin_PROGRAMS = mergesgf

mergesgf_SOURCES = \
MergeSgfMain.cpp

noinst_HEADERS =

mergesgf_LDADD = \
$(FUEGO_BUILD)/go/libfuego_go.a \
$(FUEGO_BUILD)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD)/gtpengine/libfuego_gtpengine.a \
$(BOOST_FILESYSTEM_LIB) \
$(BOOST_SYSTEM_LIB) \
$(BOOST_THREAD_LIB)

mergesgf_DEPENDENCIES = \
$(FUEGO_BUILD)/go/libfuego_go.a \
$(FUEGO_BUILD)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD)/gtpengine/libfuego_gtpengine.a

mergesgf_LDFLAGS = $(BOOST_LDFLAGS)

mergesgf_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I$(FUEGO_ROOT)/go

DISTCLEANFILES = *~
