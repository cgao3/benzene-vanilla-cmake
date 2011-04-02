This directory contains files for creating a Windows installer for Benzene
using NSIS (http://nsis.sourceforge.net)

1. Compile a Windows version of Benzene. Currently, Benzene can only be
   compiled with Cygwin. Configure Fuego with:
     env LDFLAGS="-Wl,--large-address-aware" CXXFLAGS="-O3" configure
   and Benzene with:
     env LDFLAGS="-Wl,--large-address-aware" CXXFLAGS="-O3" \
         configure --enable-optimize=no
2. Check the macros defined at the top of install.nsis. You might have to
   update the Cygwin or Boost library versions if you used newer versions,
   or the source and build directory of Benzene if you used different ones.
3. Copy the file hexgui/windows/l4j/HexGui.exe into this directory (The
   sources for HexGui are in the Benzene repository. HexGui.exe can be created
   with the command "ant l4j").
4. Install NSIS and run the command "makensis install.nsis" in this
   directory. This will create an installer named install.exe.
