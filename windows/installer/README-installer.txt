This directory contains files for creating a Windows installer for Benzene
using NSIS (http://nsis.sourceforge.net)

1. Compile a Windows version of Benzene. Currently, Benzene can only be
   compiled with Cygwin. Make sure that Fuego and Benzene are compiled
   with optimization enabled and assrtions disabled. Configure Fuego with:
     env CXXFLAGS="-O3 -DNDEBUG" ./configure
   and Benzene with:
     ./configure --enable-assert=no --enable-optimize=yes
   Check the macros defined at the top of install.nsis. You might have to
   update the Cygwin or Boost library versions if you used newer versions,
   or the source and build directory of Benzene if you used different ones.
2. Copy the file hexgui/windows/l4j/HexGui.exe into this directory (The
   sources for HexGui are in the Benzene repository. HexGui.exe can be created
   with the command "ant l4j").
3. Install NSIS and run the command "makensis install.nsis" in this
   directory. This will create an installer named install.exe.
