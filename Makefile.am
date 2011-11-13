ACLOCAL_AMFLAGS = -I m4

# Include the documentation. Automake currently does not support doxygen 
# documentation, so we use our own makefile in doc/. This just ensures
# that the docs end-up in the distribution tarball. 
EXTRA_DIST = \
doc/Makefile \
doc/doxygen.cfg \
doc/pages/doxygen.css \
doc/pages/footer.html \
doc/pages/header.html \
doc/pages/mainpage.cpp

# All projects to build in this order
SUBDIRS = \
regression \
share \
tournament \
src \
tools
