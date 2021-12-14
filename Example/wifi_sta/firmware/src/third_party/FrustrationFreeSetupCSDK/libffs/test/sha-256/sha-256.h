/*
 * Public-domain SHA-256 implementation from https://github.com/amosnier/sha-2.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void calc_sha_256(uint8_t hash[32], const void *input, size_t len);

#ifdef __cplusplus
}
#endif
