Olex2: Crystallography at your fingertip!
---
This is the repository for Olex2. The HTML GUI is kept at a different 
[Subversion repository](http://svn.olex2.org/olex2-gui). There are some binaries in the software that are present in the
GUI. It is recommended to download the official [
Olex2 distribution](https://www.olexsys.org/olex2/docs/getting-started/installing-olex2/) and replace the Olex2 and 
Unirun binaries that this code generates if you want to modify it.

## Building with CMakeLists on Linux
Bellow are build instructions on Linux using CMAKE. You will need Python 3.8. It is recommended to use a virtual 
environment, like conda, available at [Miniforge3](https://github.com/conda-forge/miniforge). 

Remember to replace everything between <> with the equivalent names for your environment and conda location
```bash
mkdir build && cd build
cmake .. -GNinja \
          -DPYTHON_LIBRARY=<conda_home>/envs/<python3.8_environment>/lib/libpython3.8.so.1.0 \
          -DPYTHON_INCLUDE_DIR=<conda_home>/envs/<python3.8_environment>/include/python3.8 \
          -DPYTHON_ROOT_DIR=<conda_home>/envs/<python3.8_environment> \
          -DPython3_FIND_STRATEGY="LOCATION"
```
If everything goes right, you can build it:
```bash
ninja -j
```
If you want to test your modifications with every build, you can use:
```bash
cmake .. -GNinja \
          -DPYTHON_LIBRARY=<conda_home>/envs/<python3.8_environment>/lib/libpython3.8.so.1.0 \
          -DPYTHON_INCLUDE_DIR=<conda_home>/envs/<python3.8_environment>/include/python3.8 \
          -DPYTHON_ROOT_DIR=<conda_home>/envs/<python3.8_environment> \
          -DPython3_FIND_STRATEGY="LOCATION" \
          -DCOPY_GUI_FILES=ON \
          -DOLEX2_GUI_DIRECTORY=<YOUR_OLEX2_GUI_DIRECTORY>
```
This way, everything will be copied, but the built files will not be overwritten.
