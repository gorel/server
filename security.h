#ifndef SECURITY_H
#define SECURITY_H

#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#define NUM_BITS 2048
#define E_VAL "65537"

/* Generate an RSA key pair */
RSA *generate_RSA_key(void);

/* Free the memory allocated by the given RSA key */
void delete_RSA_key(RSA *rsa);

/* Send a <key> key to the user for encryption of messages */
void send_session_key(char *session_key, int keylen, RSA *user_rsa_key);

#endif
