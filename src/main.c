// heavily referenced: https://stackoverflow.com/a/62546007

#include <string.h>
#include <stdio.h>

#include <gcrypt.h>

// Can encrypt up to KEY_N_BITS bits of data
#define KEY_N_BITS     600
/* Amount of digits in KEY_N_BITS (needed
for gcrypt to make an encryption key) */
#define KEY_N_BITS_LEN 3

#define FILES_DIR      "My Important Files\\"

typedef struct {
    char* fullPath;
    int dataLen;
} EncryptedFile;

static void die(const char *format, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, format);
    vfprintf(stderr, format, arg_ptr);
    va_end(arg_ptr);
    if (*format && format[strlen(format) - 1] != '\n') {
        putc('\n', stderr);
    }
    exit(1);
}

static void show_sexp(const char *prefix, gcry_sexp_t a) {
    char *buf;
    size_t size;

    if (prefix)
        fputs(prefix, stderr);
    size = gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
    buf = gcry_xmalloc(size);

    gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, buf, size);
    fprintf(stderr, "%.*s", (int) size, buf);
    gcry_free(buf);
}

// Returns an RSA encryption key of KEY_N_BITS length
gcry_sexp_t getEncryptionKey(unsigned int nBits, unsigned int nBitsLen) {
    gcry_sexp_t key_spec, key, pub_key, sec_key;

    char sexpFormat[50];
    sprintf(sexpFormat, "(genkey (rsa (nbits %u:%u)))", nBitsLen, nBits);

    int err = gcry_sexp_new(&key_spec, sexpFormat, 0, 1);
    if (err) {
        die("error creating S-expression: %s\n", gcry_strerror(err));
    }
    //>> Generate key
    err = gcry_pk_genkey(&key, key_spec);
    gcry_sexp_release(key_spec);
    if (err) {
        die("error generating RSA key: %s\n", gcry_strerror(err));
    }

    //>> Extract parts
    pub_key = gcry_sexp_find_token(key, "public-key", 0);
    if (!pub_key) {
        die("public part missing in key\n");
    }

    sec_key = gcry_sexp_find_token(key, "private-key", 0);
    if (!sec_key) {
        die("private part missing in key\n");
    }

    return key;
}

// For debugging, mostly
void copyTextFile(const char* dstFileFullPath, const char* srcFileFullPath) {
    FILE* dstFile = fopen(dstFileFullPath, "w+");
    FILE* srcFile = fopen(srcFileFullPath, "r");

    fseek(srcFile, 0, SEEK_END);
    int srcFileSize = ftell(srcFile);
    rewind(srcFile);

    unsigned char* srcFileData = (unsigned char*)malloc((sizeof(unsigned char) * srcFileSize + 1));
    fread(srcFileData, sizeof(unsigned char), srcFileSize, srcFile);
    srcFileData[srcFileSize - 1] = '\0';

    fseek(dstFile, 0, SEEK_SET);
    fwrite(srcFileData, strlen(srcFileData), 1, dstFile);

    fclose(dstFile);
    fclose(srcFile);
    free(srcFileData);
}

// Encrypt a text file of up to KEY_N_BITS length
EncryptedFile encryptTextFile(gcry_sexp_t* cipher, gcry_sexp_t pubKey, char* fullPath) {
    EncryptedFile fileInfo = {.fullPath = fullPath};
    // Open file
    FILE* filetoEncrypt = fopen(fullPath, "rb+");
    if (filetoEncrypt == NULL) {
        printf("Failed to open file \"%s\" for encryption\n", fullPath);
        fileInfo.dataLen = 0;
        return fileInfo;
    }
    fseek(filetoEncrypt, 0, SEEK_END);

    int fileSize = ftell(filetoEncrypt);
    /* Store the data length so that
    we know where to null-terminate
    the string after decryption */
    fileInfo.dataLen = fileSize;

    rewind(filetoEncrypt);

    // Read file into array
    char* fileData = (char*)malloc(sizeof(char) * fileSize);
    fread(fileData, sizeof(char), fileSize, filetoEncrypt);

    gcry_sexp_t plain, l;
    size_t len;

    printf("Original text: \n");
    for (int i = 0; i < fileSize; i++)
        printf("%c", fileData[i]);
    printf("\n\n");

    // Read fileData into an S expression
    int err;
    if (err = gcry_sexp_build(&plain, NULL, "(data (flags raw) (value %s))", fileData))
        die("converting data for encryption failed: %s\n", gcry_strerror(err));

    //>> Encrypt data
    if (err = gcry_pk_encrypt(cipher, plain, pubKey))
        die("encryption failed: %s\n", gcry_strerror(err));

    // Find encrypted data in S expression
    l = gcry_sexp_find_token(*cipher, "a", 0);
    const char *data = gcry_sexp_nth_data(l, 1, &len);

    // Write encrypted data to file
    fwrite(data, len, 1, filetoEncrypt);

    printf("Encrypted data:\n");
    for (int i = 0; i < len; i++) {
        printf("%02X", (unsigned char)data[i]);
    }
    printf("\n\n");

    fclose(filetoEncrypt);
    gcry_free(plain);
    free(fileData);

    return fileInfo;
}

// Decrypt a text file encrypted using the provided RSA private key
void decryptTextFile(gcry_sexp_t* cipher, gcry_sexp_t privKey, EncryptedFile* fileInfo) {
    // Open file
    FILE* filetoDecrypt = fopen(fileInfo->fullPath, "wb+");
    if (filetoDecrypt == NULL) {
        printf("Failed to open file \"%s\" for decryption\n", fileInfo->fullPath);
        return;
    }
    fseek(filetoDecrypt, 0, SEEK_END);

    int fileSize = ftell(filetoDecrypt);

    rewind(filetoDecrypt);

    // Read file into array
    char* fileData = (char*)malloc(sizeof(char) * fileSize);
    fread(fileData, sizeof(char), fileSize, filetoDecrypt);

    gcry_sexp_t encryptedData, tmpList, plain;
    char* decryptedData;
    
    // Read fileData into an S expression
    int err;
    if (err = gcry_sexp_build(&encryptedData, NULL, "(data (flags raw) (value %s))", fileData))
        die("converting data for decryption failed: %s\n", gcry_strerror(err));

    //>> Decrypt data
    if (err = gcry_pk_decrypt(&plain, *cipher, privKey))
        die("decryption failed: %s\n", gcry_strerror(err));

    // Find string in S expression
    tmpList = gcry_sexp_find_token(plain, "value", 0);
    if (tmpList)
        decryptedData = gcry_sexp_nth_string(tmpList, 1);
    else
        decryptedData = gcry_sexp_nth_string(plain, 0);

    // Write decrypted data to file
    fwrite(decryptedData, sizeof(char), fileInfo->dataLen, filetoDecrypt);

    printf("Decrypted data:\n%s\n\n", decryptedData);

    fclose(filetoDecrypt);
    gcry_free(decryptedData);
    gcry_free(plain);
    free(fileData);
}

int main(int argc, char* argv[]) {
    // Just for debugging
    copyTextFile(FILES_DIR"Addresses.txt",
        FILES_DIR"Addresses - copia.txt");
    copyTextFile(FILES_DIR"Credit Card Information.txt",
        FILES_DIR"Credit Card Information - copia.txt");
    copyTextFile(FILES_DIR"My Mothers Maiden Name.txt",
        FILES_DIR"My Mothers Maiden Name - copia.txt");
    copyTextFile(FILES_DIR"Passwords.txt",
        FILES_DIR"Passwords - copia.txt");
    copyTextFile(FILES_DIR"RSA Private Key.txt",
        FILES_DIR"RSA Private Key - copia.txt");

    // Initialize gcrypt library
    const char* gcryptVersion = gcry_check_version("1.10.1");
    printf("Using libgcrypt version %s\n\n", gcryptVersion);

    // Get RSA encryption key
    gcry_sexp_t key = getEncryptionKey(KEY_N_BITS, KEY_N_BITS_LEN);
    gcry_sexp_t pubKey = gcry_sexp_find_token(key, "public-key", 0);
    gcry_sexp_t privKey = gcry_sexp_find_token(key, "private-key", 0);
    gcry_sexp_t cipher;

    char* files[] = {
        FILES_DIR"Addresses.txt",
        FILES_DIR"Credit Card Information.txt",
        FILES_DIR"My Mothers Maiden Name.txt",
        FILES_DIR"Passwords.txt",
        FILES_DIR"RSA Private Key.txt"
    };
    int filesLen = sizeof(files)/sizeof(files[0]);

    EncryptedFile encryptedFiles[filesLen];

    // Encrypt files
    for (int i = 0; i < filesLen; i++) {
        encryptedFiles[i] = encryptTextFile(&cipher, pubKey, files[i]);
        decryptTextFile(&cipher, privKey, &encryptedFiles[i]);
    }

    // // Decrypt files
    // for (int i = 0; i < filesLen; i++)
    //     decryptTextFile(&cipher, privKey, &encryptedFiles[i]);

    return 0;
}