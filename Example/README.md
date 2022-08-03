# Amazon Frustration Free Setup for PIC32MZ-W1 / WFI32E01
<img src="Docs/IoT-Made-Easy-Logo.png" width=100>

#### **It is a beta relase package and requires manual code changes, the final release planned for CQ1-2022 will be much simpler**

Devices: **| PIC32 WFI32E | WFI32 | PIC32MZW1 |**

Features: **| Amazon Frustration Free Setup | Wi-Fi Setup Service (WSS) |**

## Introduction

The goal of this project is to demonstrate the [Amazon Frustration Free setup](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html) on the PIC32MZ-W1 / WFI32E01.

The Amazon Frustration Free Setup (FFS) for Wi-Fi devices is called [**Wi-Fi Simple Setup** (WSS)](https://developer.amazon.com/docs/frustration-free-setup/understand-wi-fi-simple-setup.html), it is designed to provision the new Wi-Fi devices to the Home network without any user interaction. 

The Amazon FFS (Wi-Fi Simple Setup) requires, 
- A device(PIC32MZ-W1 / WFI32E01), [pre-attested](https://developer.amazon.com/docs/frustration-free-setup/provisionee-manufacturing.html#requesting-a-dak-from-amazon) with user's Amazon Account
- A Amazon [Provisioner device](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html#testing-your-device) connected to internet
- A Home AP, who has it's credentials saved at [Amazon Wi-Fi Locker](https://www.amazon.com/gp/help/customer/display.html?nodeId=202122980) (a Amazon server)

### Hardware Requirements
- [PIC32MZ-W1 Curiosity](https://www.microchip.com/en-us/development-tool/EV12F11A) or [WFI32-IoT board](https://www.microchip.com/en-us/development-tool/EV36W50A)
- [Amazon Provisioner Device](https://developer.amazon.com/docs/frustration-free-setup/understanding-ffs.html#testing-your-device)
- Access Point with Internet

### Software Requirements
- [MPLAB X IDE](https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-x-ide) (v5.50 or later)
- [MPLAB XC32](https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-xc-compilers) (v3.01 or later)
- [MPLAB Harmony 3](https://www.microchip.com/en-us/development-tools-tools-and-software/embedded-software-center/mplab-harmony-v3)
- [python 3.x](https://www.python.org/downloads/)

	- Note: The project was developed and tested using the MPLAB X v5.50, MPLAB XC32 v2.41 and python 3.9




## Demo Setup 
The FFS demo needs an Amazon Provisionee device (Ex: Alexa Echo Dot), a Home AP, whose credentials are already saved in the Amazon Wi-Fi Locker and a PIC32MZ-W1 / WFI32E01 board running the FFS Demo.

The following diagram shows the FFS demo setup for PIC32MZ-W1 / WFI32E01.

<p align="center"><img width="600" src="Docs/FFS-Setup.png">
</p>


### Enabling WSS on PIC32MZ-W1 / WFI32E01
#### Device Attestation and Authorization

1. In order to enable FFS, the product (PIC32MZ-W1 / WFI32E01 development board) should be registered at [FFS product registration](https://developer.amazon.com/frustration-free-setup/console/v2/onboard/request-device-registration) link
2. The successful registration will provide a unique Product Type ID, Product ID and a DSS public key. Save the DSS public key in a file *device_type_pubkey.pem*
<p align="center"><img width="600" src="Docs/ffs-dev-registration-dss-pubKey.png">
</p>

3. Using these information, device specific certificates and keys can be generated. 
4. The Amazon FFS setup provides, [Device Attestation Key(DAK)](https://developer.amazon.com/frustration-free-setup/console/v2/manage-daks) which acts as Provisionee's Certificate Authority.
5. The DAK generates certificate signing request and private key pair, the csr(certificate signing request) will be signed by Amazon. 
6. In the next process, the Device Hardware Authentication (DHA) material is generated which will be signed by DAK.
7. The signed DHA certificate and private key are flashed into the Non Volatile Memory (NVM) of the device.
8. The device product ID and compressed DHA public key extracted from the device certificate should be passed to Amazon throguh the [Test device Template](https://developer.amazon.com/frustration-free-setup/console/v2/manage/submit-test-devices).
9. Amazon will register the device details into the user's Amazon account. It will be used by Amazon Provisionee to compute the SoftAP credentials.
10. Now follow the next section to add Frustration Free Setup (FFS) capability on PIC32MZ-W1 / WFI32E01

## Running the FFS example project

The example project demonstrates the FFS on WFI32-IoT platform. Please follow the steps below to run the FFS example project.

#### Using DHA in PIC32MZ-W1 / WFI32E01 FFS Project
1. The "Device Attestation and Authorization" steps would result in following files
	-  dak.conf
	-  dak-params.pem
	-  dak.csr
	-  dak_private_key.pem
	-  dak-certificate-xxxxxx.pem
	-  device.conf
	-  device-params.pem
	-  device.csr
	-  private_key.pem
	-  **device-certificate.pem**
	-  certificate.pem
	-  dha-control-log-public-key.txt
    -  **device_type_pubkey.pem**

2. Choose the PIC32MZ-W1 MPLAB Harmoney 3 project to which the FFS capability is needed, we suggest to start with *paho_mqtt_tls_client* project.
3. Checkout the [PIC32MZ-W1 FreeRTOS FFS](https://github.com/MicrochipTech/pic32mzw1_ffs_amazon_freertos.git) repo in the project's *../firmware/src* folder
4. Copy the **private_key**, **certificate.pem** and **device_type_pubkey.pem** into the cloned repo *tools* folder.
5. Install the certificate creation python script requirements using the *pip3 install -r requirements.txt*
<p align="center"><img width="600" src="Docs/ffs-python-requirements.png">
</p>

6. Run the *create-ffs-msd-files.py -r [SRootCA.cer](https://ssl-ccp.secureserver.net/repository/sf-class2-root.crt) -c **device-certificate.pem** -k **private_key.pem** -t **device_type_pubkey.pem*** command, it will generate 3 certificate files.

	- ffsRootCA.cer
	- ffsDevPublic.key
	- ffsDevTypePublic.key

<p align="center"><img width="600" src="Docs/ffs-cert-script-cmd.png">
</p>
 
7. Now we have all the files necessory to configure/enable the FFS

8. The WFI32-IoT emulates a MSD(Mass Storage Devcie) while running the demo for the first time.

<p align="center"><img width="600" src="Docs/first_boot_log.png">
</p>

9. Copy all the above generated files into the MSD drive. The above log provides the list of files needed to enable FFS.

<p align="center"><img width="600" src="Docs/MSD_for_certs.png">
</p>



## Known issues and Limitations

 

## FAQ
1. **Can FFS demo work with any Amazon Provisioner device?**

	No, the default FFS demo certificates are linked to  Amazon user's account. The demo will work only with those Amazon Provisioner devices which are logged in with same user's credentials

2. **Can FFS demo work with a Amazon Provisioner device connected to 5GHz router?**

	No, the Amazon Provisioner disables 2.4Ghz when it is connected to 5GHz AP so, the PIC32MZ-W1 would fail to connect to Provisioner device as it only support 2.4GHz.






