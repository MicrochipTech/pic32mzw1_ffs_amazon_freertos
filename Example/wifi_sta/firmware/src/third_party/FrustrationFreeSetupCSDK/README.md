# Building
1. Create the build system:
```
mkdir build
cd build
cmake ..
```
Debug and unit tests are enabled by default. To disable either, set `ENABLE_DEBUG` or `ENABLE_TESTS` to `OFF` in `CMakeLists.txt`.
2. To build and install the library use `sudo make install` from the build directory.
Use `make` if you just want to build it but do not want to install and expose it for others.
If there are no changes to the build system itself, this is the only step required when making new changes.
3. To generate Doxygen documentation, install Doxygen with `sudo apt-get install doxygen`.
Run `doxygen doxygen/Doxyfile` from the package's root directory.

# Running the tests
1. Build the library.
2. Run `./libffs/test/all_tests` from the build directory.

# Building and running the demo
1. Build the library.
2. `cd` to the `ffs_linux` directory
1. Create the build system:
```
mkdir build
cd build
cmake ..
```
Debug and unit tests are enabled by default. To disable either, set `ENABLE_DEBUG` or `ENABLE_TESTS` to `OFF` in `CMakeLists.txt`.
2. Build with `make` from the build directory.
3. Run the demo executable with sudo permissions: `sudo ./FrustrationFreeSetupLinuxDemo
