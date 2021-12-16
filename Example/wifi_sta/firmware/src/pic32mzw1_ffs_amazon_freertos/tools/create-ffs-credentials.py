from string import Template
import subprocess
import binascii
from os import pipe, write
import sys, getopt
from  cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography import x509

LICENSE_BANNER_FILE = "mchp_license_banner.txt"


ffsCertificateHeaderFile=Template("""
$licenseBanner

/*GENERATED FILE*/
#ifndef _AMAZON_FFS_PIC32MZW1_H_
#define _AMAZON_FFS_PIC32MZW1_H_

static const unsigned char device_public_key_der[] =
{
$devicePubKey
};
/* size is $devicePubKeySize */
static const int sizeof_device_public_key_der = sizeof(device_public_key_der);


static const unsigned char device_type_public_key_der[] =
{
$deviceTypepubKey
};
/* size is $deviceTypePubKeySize */
static const int sizeof_device_type_public_key_der = sizeof(device_type_public_key_der);


static const unsigned char devicePvtKey[] =
{
$devicePrivKey
};
/* size is $devicePrivKeySize */
static const int devicePvtKey_len = sizeof(devicePvtKey);

static const unsigned char caCert[] =
{
$amazonFfsDssCA
};
/* size is $amazonFfsDssCASize */
static const int caCert_len = sizeof(caCert);


static const unsigned char deviceCert[] =
{
$deviceCertificate
};
/* size is $deviceCertificateSize */
static const int deviceCert_len = sizeof(deviceCert);

#endif /*_AMAZON_FFS_PIC32MZW1_H_*/
""")



def make_sublist_group(lst: list, grp: int) -> list:
    """
    Group list elements into sublists.
    make_sublist_group([1, 2, 3, 4, 5, 6, 7], 3) = [[1, 2, 3], [4, 5, 6], 7]
    """
    return [lst[i:i+grp] for i in range(0, len(lst), grp)]

def do_convension(content: bytes) -> str:
    hexstr = binascii.hexlify(content).decode("UTF-8")
    hexstr = hexstr.upper()
    array = ["0x" + hexstr[i:i + 2] + "" for i in range(0, len(hexstr), 2)]
    array = make_sublist_group(array, 10)
    
    return sum(len(a) for a in array ), "\n".join([", ".join(e) + ", " for e in array])		

def main(argv):
	ffsRootCa = 'SRootCA.cer'   	
	ffsDevTypePubKey = 'device-type-public.key'
	ffsDevCert = 'device-certificate.pem'
	ffsDevPrivKey = 'private_key.pem'
	try:
		opts, args = getopt.getopt(argv,"hr:c:k:t:",["ca=","cert=","key=","type="])
	except getopt.GetoptError:
		print('create-ffs-credentials.py -r <root CA> -c <device certificate> -k <device private key> -t <device type public key>')
		sys.exit(2)
	
	for opt, arg in opts:
		if opt == '-h':
			print('create-ffs-credentials.py -r <root CA> -c <device certificate> -k <device key> -t <device type public key>')
			sys.exit()
		elif opt in ("-r", "--ca"):
			ffsRootCa = arg
		elif opt in ("-c", "--cert"):		
			ffsDevCert = arg
		elif opt in ("-k", "--key"):		  
			ffsDevPrivKey = arg		 		
		elif opt in ("-t", "--type"):		  
			ffsDevTypePubKey = arg
	print('Root CA file is ', ffsRootCa)
	print('Device Certificate file is ', ffsDevCert)
	print('Device Private Key file is ', ffsDevPrivKey)	
	print('Device Type Public file is ', ffsDevTypePubKey)
	print("-------Root CA-------")
	##Amazon FFS DSS CA
	with open(ffsRootCa, 'rb') as fHdl:
		try:
			certHdl = x509.load_der_x509_certificate(fHdl.read())
			caCertArray = certHdl.public_bytes(serialization.Encoding.DER)						
			caCertBytesLen, caCertBytes = do_convension(caCertArray)				
		except Exception as e:
			print(e)
	
	print("-------Pub Key-------")
	with open(ffsDevCert, 'rb') as fHdl:
		try:
			certHdl = x509.load_pem_x509_certificate(fHdl.read())
			pubKeyHdl = certHdl.public_key()			
			pubKeyBytes = pubKeyHdl.public_bytes(serialization.Encoding.DER,
				serialization.PublicFormat.SubjectPublicKeyInfo)			
			pubKeyLen, pubKeyArray = do_convension(pubKeyBytes)			
		except Exception as e:
			print(e)
			return
	
	print("-------Dev Cert-------")
	with open(ffsDevCert, 'r') as certFile:
		devCert = certFile.read().replace('\n', '\\\r\n')		
		devCert = '"' + devCert + '"'
		devCertLen = len(devCert)
		

	print("-------Dev Type Pub Key-------")
	##Device Type public key
	with open(ffsDevTypePubKey, 'rb') as fHdl:
		try:
			devTypePubKeyHdl = serialization.load_pem_public_key(fHdl.read(), default_backend())
			devTypePubKeyBytes = devTypePubKeyHdl.public_bytes(serialization.Encoding.DER, 
			serialization.PublicFormat.SubjectPublicKeyInfo)
			devTypePubKeyLen, devTypePubKeyArray = do_convension(devTypePubKeyBytes)								
		except Exception as e:
			print(e)

	##Device private key
	with open(ffsDevPrivKey, "rb") as keyfile:
		pvtKeyHdl = serialization.load_pem_private_key(
		keyfile.read(),
		password=None,
		backend=default_backend()
		)

		privKeyBytes = pvtKeyHdl.private_bytes(
			serialization.Encoding.DER,
			serialization.PrivateFormat.TraditionalOpenSSL,
			serialization.NoEncryption()
		)
		privKeyLen , privKeyArray = do_convension(privKeyBytes)
			


	with open("../app/amazon_ffs_certs.h", 'w') as file:
		with open(LICENSE_BANNER_FILE, 'r') as linceseFile:
			file.write(ffsCertificateHeaderFile.substitute(licenseBanner=linceseFile.read(), 
			devicePubKey=pubKeyArray,devicePubKeySize=pubKeyLen, 
			devicePrivKey=privKeyArray,devicePrivKeySize=privKeyLen, 
			deviceTypepubKey=devTypePubKeyArray, deviceTypePubKeySize=devTypePubKeyLen, 
			deviceCertificate=devCert, deviceCertificateSize=devCertLen,
			amazonFfsDssCA=caCertBytes, amazonFfsDssCASize = caCertBytesLen
			))


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
