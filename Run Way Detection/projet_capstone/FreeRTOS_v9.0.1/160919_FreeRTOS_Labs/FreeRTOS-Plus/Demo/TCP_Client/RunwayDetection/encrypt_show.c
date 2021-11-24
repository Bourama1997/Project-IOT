#include <stdlib.h>
#include <string.h>
#include "../../Library/console.h"

#include "encrypt_show.h"

void encrypt_show(key_pair_t key_pair, const char* message)
{
    unsigned short i;
    size_t message_length = strlen(message);

    long
        * unencrypted_message_wrapper = calloc(message_length, sizeof(long)),
        * encrypted_message_wrapper = calloc(message_length, sizeof(long));

    char* result = calloc(message_length, sizeof(char));

    log("unencrypted demo message: \"%s\"", message);

    for (i = 0; i < message_length; ++i)
        unencrypted_message_wrapper[i] = message[i];

    encrypt(
        encrypted_message_wrapper,
        unencrypted_message_wrapper,
        key_pair.public_key,
        key_pair.rsa_modulus,
        message_length,
        0, 1
    );

    for (i = 0; i < message_length; ++i)
        result[i] = encrypted_message_wrapper[i];
    result[i] = '\0';

    log("encrypted demo message: \"%s\"", result);

    decrypt(
        encrypted_message_wrapper,
        encrypted_message_wrapper,
        key_pair.private_key,
        key_pair.rsa_modulus,
        message_length,
        0, 1
    );

    for (i = 0; i < message_length; ++i)
        result[i] = encrypted_message_wrapper[i];
    result[i] = '\0';

    log("decrypted demo message: \"%s\"", result);

    free(result);
    free(encrypted_message_wrapper);
    free(unencrypted_message_wrapper);
}
