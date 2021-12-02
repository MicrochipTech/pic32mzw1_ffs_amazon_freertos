# pic32mzw1_ffs_amazon_freertos
Amazon Frustration Free Setup for PIC32MZW1

## Introduction

The goal of this project is to demonstrate the [Amazon Frustation Free setup](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html) on the PIC32MZW1.

The Amazon Frustation Free Setup(FFS) for WiFi devices is called [**Wi-Fi Simple Setup**(WSS)](https://developer.amazon.com/docs/frustration-free-setup/understand-wi-fi-simple-setup.html), it is designed to provision the new Wi-Fi devices to the Home network without any user interation. 

The Amazon FFS(Wi-Fi Simple Setup) requires, 
- A device [pre-attested](https://developer.amazon.com/docs/frustration-free-setup/provisionee-manufacturing.html#requesting-a-dak-from-amazon) to users Amazon Account
- At least one [Amazon Provisionee device](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html#testing-your-device) connected to internet
- Wi-Fi credentials must be available at Wi-Fi Locker 

## Hardware Requirements
- PIC32MZW1 Curiosity or WFI32-IoT board
- [Amazon Provisioner Device](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html#testing-your-device)
- Access Point

## Software Requirements
- MPLABX
- XC32
- Harmony3
- H3 PIC32MZW1 freertos project 


## Demo Setup 
The folloowing diagram shows the FFS demo setup for PIC32MZW1.

<p align="center"><img src="Docs/FFS-Setup.png">
</p>

On power-up, the FFS capable device looks for available Amazon Provisionee device in the vicinity, then the provisioner comes up as a hidden secured SoftAP and lets the provisionee device to contact to it. On successful connection, the provisionee establishes a secured HTTP connection with Device Setup(DSS) Service and shares the product details. The DSS will associate the device to user account. Now the provisionee will scan and share the available access pointes with DSS. The DSS would look for a match in the users Amazon Wi-Fi Locker, and provides the credentials for the matching AP.

<p align="center"><img src="Docs/WSS-FlowDiagram.png">
</p>



## Enabling WSS on PIC32MZW1 
#### Device Attestation and Authorization

1. In order enable FFS on any product, the product should be registered at [FFS product registration](https://developer.amazon.com/frustration-free-setup/console/v2/onboard/request-device-registration)
2. The successful registration would enable to generate devcie certificates and keys
3. The FFS setup provides, [Device Attestation Key(DAK)](https://developer.amazon.com/frustration-free-setup/console/v2/manage-daks) which acts as a Certificate Authority for a device type
4. The DAK generates certificate signing request and private key pair, the csr(certificate signing request) will be signed by Amazon. 
5. In the next process, the Device Hardware Authentication(DHA) material is generated and signed by the DAK.
6. The signed DHA certificate and private key are flashed into the NVM of the device
7. The DHA public key is extracted from the certificate and shared with amazon through a control log end point
8. Amazon would register the device in the user's Amazon account 
9. Now the device is ready for the Frustration Free Setup
#### Using DHA in PIC32MZW1 FFS Project
1. The above steps would result in following files
	-  dak.conf
	-  dak-params.pem
	-  dak.csr
	-  dak_private_key.pem
	-  dak-certificate-xxxxxx.pem
	-  device.conf
	-  device-params.pem
	-  device.csr
	-  private_key.pem
	-  device-certificate.pem
	-  certificate.pem
	-  dha-control-log-public-key.txt
2. Copy the create.py python file to the same folder containing above files
3. Run the following command and it will generate the certificate file to be used with MHC
4. Clone the pic32mzw1_ffs_amazon_freertos





Contact [microchip-ffs-support@microchip.com](mailto:microchip-ffs-support@microchip.com) for support