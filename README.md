# pic32mzw1_ffs_amazon_freertos
Amazon Frustration Free Setup for PIC32MZW1

## Introduction

The goal of this project is to demonstrate the [Amazon Frustation Free setup](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html) on the PIC32MZW1.The Amazon Frustation Free Setup(FFS) is designed to provision the new Wi-Fi devices to the Home network without any user interation. The Amazon FFS has 2 methods,

- Zero-Touch Setup
- Barcode Setup

Note:- We only cover the Zero-Touch Setup, the Barcode Setup needs a mobile application to attest the device Amazon User account which is not in the scope of the project.
 

## Hardware Requirements
- PIC32MZW1 Curiosity or WFI32-IoT board
- [Amazon Provisioner Device](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html#testing-your-device)
- Access Point

## Software Requirements
- MPLABX
- xc32
- Harmony3



## Demo Setup 
<p align="center"><img src="Docs/FFS-BlockDiagram.png">
</p>


The Amazon FFS allows

## Developer Steps
- Register the Device at 

Contact [microchip-ffs-support@microchip.com](mailto:microchip-ffs-support@microchip.com) for support