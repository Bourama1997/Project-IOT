#ifndef RSA_H
#define RSA_H

/*#ifndef RSA_PRIME_P
#define RSA_PRIME_P 167
#endif

#ifndef RSA_PRIME_Q
#define RSA_PRIME_Q 223
#endif*/

typedef struct key_pair
{
    long rsa_modulus;
    long private_key;
    long public_key;
} key_pair_t;

key_pair_t generate_key_pair(long prime_p, long prime_q);

void encrypt(
    long encrypted_data[],
    long unencrypted_data[],
    long public_key,
    long rsa_modulus,
    unsigned long data_array_size,
    unsigned short log_result,
    unsigned short is_text
);

long find_prime(
    long prime_p,
    const char message[],
    const char expected_encrypted_message[]
);

void decrypt(
    long encrypted_data[],
    long decrypted_data[],
    long private_key,
    long rsa_modulus,
    unsigned long data_array_size,
    unsigned short log_result,
    unsigned short is_text
);

#endif /* RSA_H */
