#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rsa.h"

static int is_prime_number(long num, long* prime_num_sqrt)
{
    unsigned long i;
    *prime_num_sqrt=(long)sqrt((double)num);
    for(i=2;i<=(unsigned long)*prime_num_sqrt;i++)
    {
        if(num%i==0)
            return 0;
    }
    return 1;
}

static long cd(long x, long t)
{
    long k = 1;
    while (1)
    {
        k = k + t;
        if (k % x == 0)
            return(k / x);
    }
}

static void ce(
    long* prime_num_sqrt,
    long* private_key,
    long* public_key,
    long prime_q,
    long prime_p
)
{
    unsigned long i;
    long t = (prime_p - 1) * (prime_q - 1);

    for(i=2;i<(unsigned long)t;i++)
    {
        if(t%i==0)
            continue;
        long is_prime=is_prime_number(i, prime_num_sqrt);
        if(is_prime==1&&i!=(unsigned long)prime_p&&i!=(unsigned long)prime_q)
        {
            *public_key=i;
            is_prime=cd(*public_key, t);
            if(is_prime>0)
            {
                *private_key=is_prime;
                break;
            }
        }
    }
}

key_pair_t generate_key_pair(
    long prime_p,
    long prime_q
) {
    long
        rsa_modulus = prime_p * prime_q,
        prime_num_sqrt;

    key_pair_t key_pair;
    key_pair.rsa_modulus = rsa_modulus;

    ce(
        &prime_num_sqrt,
        &key_pair.private_key,
        &key_pair.public_key,
        prime_q,
        prime_p
    );

    //printf("\nGENERATED PRIVATE KEY WITH p = %ld AND q = %ld\n", prime_p, prime_q);

    /*printf(
        "\nRSA modulus = %ld\nPublic key = %ld\nPrivate key = %ld\n",
        key_pair.rsa_modulus,
        key_pair.public_key,
        key_pair.private_key
    );*/

    return key_pair;
}

void encrypt(
    long encrypted_data[],
    long unencrypted_data[],
    long public_key,
    long rsa_modulus,
    unsigned long data_array_size,
    unsigned short log_result,
    unsigned short is_text
) {
    long pt,ct,k;
    unsigned long i=0;
    while(i!=data_array_size)
    {
        pt=unencrypted_data[i];
        
        if (is_text)
            pt=pt-96;

        k=1;
        unsigned long j;
        for(j=0;j<(unsigned long)public_key;j++)
        {
            k=k*pt;
            k=k%rsa_modulus;
        }
        ct=k;

        if (is_text)
            ct = ct + 96;

        encrypted_data[i]=ct;
        i++;
    }

    if (log_result)
    {
        printf("\nTHE ENCRYPTED MESSAGE IS\n");
        for(i=0;i<data_array_size;i++)
            printf("%c",encrypted_data[i]);
    }
}

long find_prime(
    long prime_p,
    const char message[],
    const char expected_encrypted_message[]
) {
    unsigned long i;
    unsigned short message_length = (unsigned short)strlen(message);

    long* unencrypted_message_in_chunks = calloc(message_length, sizeof(long));
    
    for (i = 0; i < message_length; ++i)
        unencrypted_message_in_chunks[i] = message[i];

    long* encrypted_message_in_chunks = calloc(message_length, sizeof(long));
    char* encrypted_message = calloc(message_length, sizeof(char));

    long prime_q = -1;

    for (i = 2; prime_q == -1 && i != 0; ++i)
    {
        key_pair_t key_pair = generate_key_pair(prime_p, i);
        
        encrypt(
            encrypted_message_in_chunks,
            unencrypted_message_in_chunks,
            key_pair.public_key,
            key_pair.rsa_modulus,
            message_length,
            1, // log_result = true
            1 // is_text = true
        );

        unsigned short j;
        for (j = 0; j < message_length; ++j)
            encrypted_message[j] = (char)encrypted_message_in_chunks[j];
        encrypted_message[j] = '\0';

        if (strcmp(encrypted_message, expected_encrypted_message) == 0)
            prime_q = i;
    }

    free(encrypted_message);
    free(encrypted_message_in_chunks);
    free(unencrypted_message_in_chunks);

    return prime_q;
}

void decrypt(
    long encrypted_data[],
    long decrypted_data[],
    long private_key,
    long rsa_modulus,
    unsigned long data_array_size,
    unsigned short log_result,
    unsigned short is_text
) {
    long ct,k;
    unsigned long i;

    for (i = 0; i < data_array_size; i++)
    {
        ct = encrypted_data[i];

        if (is_text)
            ct = ct - 96;
        
        k = 1;
        unsigned long j;

        for (j = 0; j < (unsigned long)private_key; j++)
            k = (k * ct) % rsa_modulus;

        decrypted_data[i] = k;

        if (is_text)
            decrypted_data[i] = decrypted_data[i] + 96;
    }

    if (log_result)
    {
        printf("\nTHE DECRYPTED MESSAGE IS\n");
        for(i=0;i<data_array_size;i++)
            printf("%c", decrypted_data[i]);
    }
}
