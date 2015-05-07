/**
 * @file mega/utils.h
 * @brief Mega SDK various utilities and helper classes
 *
 * (c) 2013-2014 by Mega Limited, Auckland, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#ifndef MEGA_UTILS_H
#define MEGA_UTILS_H 1

#include "types.h"

namespace mega {
// convert 2...8 character ID to int64 integer (endian agnostic)
#define MAKENAMEID2(a, b) (nameid)(((a) << 8) + (b))
#define MAKENAMEID3(a, b, c) (nameid)(((a) << 16) + ((b) << 8) + (c))
#define MAKENAMEID4(a, b, c, d) (nameid)(((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define MAKENAMEID5(a, b, c, d, e) (nameid)((((uint64_t)a) << 32) + ((b) << 24) + ((c) << 16) + ((d) << 8) + (e))
#define MAKENAMEID6(a, b, c, d, e, f) (nameid)((((uint64_t)a) << 40) + (((uint64_t)b) << 32) + ((c) << 24) + ((d) << 16) + ((e) << 8) + (f))
#define MAKENAMEID7(a, b, c, d, e, f, g) (nameid)((((uint64_t)a) << 48) + (((uint64_t)b) << 40) + (((uint64_t)c) << 32) + ((d) << 24) + ((e) << 16) + ((f) << 8) + (g))
#define MAKENAMEID8(a, b, c, d, e, f, g, h) (nameid)((((uint64_t)a) << 56) + (((uint64_t)b) << 48) + (((uint64_t)c) << 40) + (((uint64_t)d) << 32) + ((e) << 24) + ((f) << 16) + ((g) << 8) + (h))

struct MEGA_API ChunkedHash
{
    static const int SEGSIZE = 131072;

    static m_off_t chunkfloor(m_off_t);
    static m_off_t chunkceil(m_off_t);
};

/**
 * @brief Padded encryption using AES-128 in CBC mode.
 */
struct MEGA_API PaddedCBC
{
    /**
     * @brief Encrypts a string after padding it to block length.
     *
     * Note: With an IV, only use the first 8 bytes.
     *
     * @param data Data buffer to be encrypted. Encryption is done in-place,
     *     so cipher text will be in `data` afterwards as well.
     * @param key AES key for encryption.
     * @param iv Optional initialisation vector for encryption. Will use a
     *     zero IV if not given. If `iv` is a zero length string, a new IV
     *     for encryption will be generated and available through the reference.
     * @return Void.
     */
    static void encrypt(string* data, SymmCipher* key, string* iv = NULL);

    /**
     * @brief Decrypts a string and strips the padding.
     *
     * Note: With an IV, only use the first 8 bytes.
     *
     * @param data Data buffer to be decrypted. Decryption is done in-place,
     *     so plain text will be in `data` afterwards as well.
     * @param key AES key for decryption.
     * @param iv Optional initialisation vector for encryption. Will use a
     *     zero IV if not given.
     * @return Void.
     */
    static bool decrypt(string* data, SymmCipher* key, string* iv = NULL);
};

class MEGA_API HashSignature
{
    Hash* hash;

public:
    // add data
    void add(const byte*, unsigned);

    // generate signature
    unsigned get(AsymmCipher*, byte*, unsigned);

    // verify signature
    bool check(AsymmCipher*, const byte*, unsigned);

    HashSignature(Hash*);
    ~HashSignature();
};

/**
 * @brief Crypto functions related to payments
 */
class MEGA_API PayCrypter
{
    /**
     * @brief Length of the AES key
     */
    static const int ENC_KEY_BYTES = 16;

    /**
     * @brief Lenght of the key to generate the HMAC
     */
    static const int MAC_KEY_BYTES = 32;

    /**
     * @brief Length of the IV for AES-CBC
     */
    static const int IV_BYTES = 16;

    /**
     * @brief Buffer for the AES key and the HMAC key
     */
    byte keys[ENC_KEY_BYTES+MAC_KEY_BYTES];

    /**
     * @brief Pointer to the buffer with the AES key
     */
    byte *encKey;

    /**
     * @brief Pointer to the buffer with the HMAC key
     */
    byte *hmacKey;

    /**
     * @brief Buffer with the IV for AES-CBC
     */
    byte iv[IV_BYTES];

public:

    /**
     * @brief Constructor. Initializes keys with random values.
     */
    PayCrypter();

    /**
     * @brief Updates the crypto keys (mainly for testing)
     * @param newEncKey New AES key (must contain ENC_KEY_BYTES bytes)
     * @param newHmacKey New HMAC key (must contain MAC_KEY_BYTES bytes)
     * @param newIv New IV for AES-CBC (must contain IV_BYTES bytes)
     */
    void setKeys(byte *newEncKey, byte *newHmacKey, byte *newIv);

    /**
     * @brief Encrypts the cleartext and returns the payload string.
     *
     * The clear text is encrypted with AES-CBC, then a HMAC-SHA256 is generated for (IV + ciphertext)
     * and finally returns (HMAC + IV + ciphertext)
     *
     * @param cleartext Clear text to generate the payload
     * @param result The function will fill this string with the generated payload
     * @return True if the funcion succeeds, otherwise false
     */
    bool encryptPayload(string *cleartext, string *result);

    /**
     * @brief Encrypts the cleartext using RSA with random padding.
     *
     * A 2-byte header is inserted just before the clear text with the size in bytes.
     * The result is padded with random bytes. Then RSA is applied and the result is returned
     * in the third parameter, with a 2-byte header that contains the size of the result of RSA.
     *
     * @param cleartext Clear text to encrypt with RSA
     * @param base64pubk Public key in Base64 format (result of AsymmCipher::serializekey and Base64::btoa)
     * @param randompadding Enables padding with random bytes. Otherwise, the cleartext is 0-padded
     * @param result RSA encrypted text, with a 2-byte header with the size of the RSA buffer in bytes
     * @return True if the funcion succeeds, otherwise false
     */
    bool rsaEncryptKeys(string *cleartext, char *base64pubk, string *result, bool randompadding = true);

    /**
     * @brief Encrypts clear text data to an authenticated ciphertext, authenticated with an HMAC.
     * @param cleartext Clear text as byte string
     * @param base64pubk Public key in Base64 format (result of AsymmCipher::serializekey and Base64::btoa)
     * @param randompadding Enables padding with random bytes. Otherwise, the cleartext is 0-padded
     * @param result Encrypted data block as byte string.
     * @return True if the funcion succeeds, otherwise false
     */
    bool hybridEncrypt(string *cleartext, char *base64pubk, string *result, bool randompadding = true);
};

// read/write multibyte words
struct MEGA_API MemAccess
{
#ifdef NO_DIRECT_WORD_ACCESS
    template<typename T> static T get(const char* ptr)
    {
        T val;
        memcpy(&val,ptr,sizeof(T));
        return val;
    }

    template<typename T> static void set(byte* ptr, T val)
    {
        memcpy(ptr,&val,sizeof val);
    }
#else
    template<typename T> static T get(const char* ptr)
    {
        return *(T*)ptr;
    }

    template<typename T> static void set(byte* ptr, T val)
    {
        *(T*)ptr = val;
    }
#endif
};
} // namespace

#endif
