from os import pipe, write
import sys, getopt, os
from  cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography import x509

def main(argv):
	ffsRootCa = 'SRootCA.cer'   	
	ffsDevTypePubKey = 'device_type_pubkey.pem'
	ffsDevCert = 'certificate.pem'	
	try:
		opts, args = getopt.getopt(argv,"hr:c:k:t:",["ca=","cert=","key=","type="])
	except getopt.GetoptError:
		print('create-ffs-credentials.py -r <root CA> -c <device certificate> -t <device type public key>')
		sys.exit(2)
	
	for opt, arg in opts:
		if opt == '-h':
			print('create-ffs-credentials.py -r <root CA> -c <device certificate> -t <device type public key>')
			sys.exit()
		elif opt in ("-r", "--ca"):
			ffsRootCa = arg
		elif opt in ("-c", "--cert"):		
			ffsDevCert = arg
		elif opt in ("-t", "--type"):		  
			ffsDevTypePubKey = arg
	print('Root CA file is ', ffsRootCa)
	print('Device Certificate file is ', ffsDevCert)	
	print('Device Type Public file is ', ffsDevTypePubKey)
	print("-------Root CA-------")
	if(os.path.isdir("./msd") == False):
		os.mkdir("./msd")
    ##Amazon FFS DSS CA
	with open(ffsRootCa, 'rb') as fHdl:
		try:
			certHdl = x509.load_der_x509_certificate(fHdl.read())
			caCertArray = certHdl.public_bytes(serialization.Encoding.DER)
			with open("./msd/ffsRootCa.der", 'wb') as file:
				file.write(caCertArray)				
		except Exception as e:
			print(e)


	print("-------Pub Key-------")
	with open(ffsDevCert, 'rb') as fHdl:
		try:
			certHdl = x509.load_pem_x509_certificate(fHdl.read())
			pubKeyHdl = certHdl.public_key()			
			pubKeyBytes = pubKeyHdl.public_bytes(serialization.Encoding.DER,
			serialization.PublicFormat.SubjectPublicKeyInfo)			
			with open("./msd/ffsDevPublic.key", 'wb') as file:
				file.write(pubKeyBytes)	
		except Exception as e:
			print(e)			
	
	print("-------Dev Type Pub Key-------")
	##Device Type public key
	with open(ffsDevTypePubKey, 'rb') as fHdl:
		try:
			devTypePubKeyHdl = serialization.load_pem_public_key(fHdl.read(), default_backend())
			devTypePubKeyBytes = devTypePubKeyHdl.public_bytes(serialization.Encoding.DER, 
			serialization.PublicFormat.SubjectPublicKeyInfo)
			with open("./msd/ffsDevTypePublic.key", 'wb') as file:
				file.write(devTypePubKeyBytes)	
		except Exception as e:
			print(e)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
