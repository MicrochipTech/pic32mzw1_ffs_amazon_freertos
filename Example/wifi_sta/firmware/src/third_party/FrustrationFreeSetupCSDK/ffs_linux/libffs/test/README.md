# Building
1. Clone the following dependencies:
`git clone ssh://git.amazon.com/pkg/FfsCommonC`
`git clone ssh://git.amazon.com/pkg/FfsDssClientC`
`git clone ssh://git.amazon.com/pkg/FfsWifiCSDK`
Follow the README in each package to build and install them.
They will also guide through installing the CMake and GTest dependencies.
2. Create the build system:
```
mkdir build
cd build
cmake ..
```
Debug and unit tests are enabled by default. To disable either, set `ENABLE_DEBUG` or `ENABLE_TESTS` to `OFF` in `CMakeLists.txt`.
3. To build and install the library use `sudo make install` from the build directory.
Use `make` if you just want to build it but do not want to install and expose it for others.
If there are no changes to the build system itself, this is the only step required when making new changes.
4. To generate Doxygen documentation, install Doxygen with `sudo apt-get install doxygen`.
Run `doxygen doxygen/Doxyfile` from the package's root directory.

# Running the tests
1. Build the library.
2. Run `./ffs/test/all_tests` from the build directory.
