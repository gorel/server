#include "security.h"

/* Generate an RSA key pair */
RSA *generate_RSA_key(void)
{
	//Allocate memory for the RSA key pair
	RSA *rsa = RSA_new();
	
	//Generate the actual RSA key pair values
	RSA_generate_key_ex(rsa, NUM_BITS, BN_dec2bn(E_VAL), NULL);
	
	//Return the RSA key pair object
	return rsa;
}

/* Free the memory allocated by the given RSA key */
void delete_RSA_key(RSA *rsa)
{
	RSA_free(rsa);
}

/* Send a <key> key to the user for encryption of messages */
unsigned char *encrypt_session_key(RSA *user_rsa_key, unsigned char *session_key, int keylen)
{
	//Allocate enough memory to hold the encrypted session key
	unsigned char *encrypted_session_key = (unsigned char *)malloc(RSA_size(user_rsa_key));

	//Encrypt the session key
	RSA_public_encrypt(keylen, session_key, encrypted_session_key, user_rsa_key, RSA_PKCS1_PADDING);
	
	//Return the encrypted session key
	return encrypted_session_key;
}
