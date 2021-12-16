# Frustration Free Setup For Espressif
## Package is under development. It may not work as intended for now.

The is going to be reference implementations of Frustration Free Setup provisionee and provisioner SDKs for IOT devices that run Amazon Free RTOS.

## Setup environment
1. Tools required:
    - python3.7
    - cmake
If you do not have any of the tools mentioned above installed, please
install them.

1. Download the Espressif toolchain. Follow the instructions outlines on this [guide](https://docs.espressif.com/projects/esp-idf/en/v3.3/get-started-cmake/macos-setup.html). 
Make sure the path is set properly after downloading the toolchain.

1. Setup certificates
Open `PEMfileToCString` and change your pem files to cstrings, and paste the strings at `espressif_app/configs/ffs_amazon_freertos_credentials.h`.

1. Download Amazon FreeRTOS to `freertos` folder:
```
git clone --branch 202002.00 https://github.com/aws/amazon-freertos.git --recurse-submodules freertos
```

1. Create build files for AmazonFree RTOS:
```
cmake -S . -B build
```

1. To build the flash binary:
```
cmake --build build -j4
```
j4 flag is going to build the flash binary in 4 threads, so it will be fast.
This will generate the build to flash on your espressif device.

1. To monitor the Esp chip:
```
idf.py monitor
```

## Development cycle
1. Make changes to your code. Make sure any new files added are put into the cmake file if necessary.
1. Compile code and flash:
```
cmake --build build -j4 --target flash
```
If you meet some troubles or want to rebuild FreeRTOS, remove the build folder and rebuild:
```
sudo rm -rf build; cmake -S . -B build
```
Combine them with the monitor command:
```
sudo rm -rf build; cmake -S . -B build && cmake --build build -j4 --target flash && idf.py monitor
```
1. Inspect logs:
```
./freertos/vendors/espressif/esp-idf/tools/idf.py monitor
```

## Troubleshooting
#### ESP device is not being detected via USB port
Run the following command:
```
ls /dev/tty.*
```
If you do not see a /dev/tty.SLAB_USBtoUART in the list, you need to install
the USB to serial drivers.
Go to this link and install the drivers:
https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

#### The wrong device is being picked up for flashing
Sometimes the terminal can pick up the wrong tty device to flash. To work around
this issue, do the following:
```
export ESPPORT=/dev/tty.SLAB_USBtoUART
```