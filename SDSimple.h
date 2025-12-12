#ifndef SD_SIMPLE
#define SD_SIMPLE

#include "hardware/spi.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <malloc.h>

uint32_t getTotalHeap(void)
{
    extern char __StackLimit, __bss_end__;

    return &__StackLimit - &__bss_end__;
}

uint32_t getFreeHeap(void)
{
    struct mallinfo m = mallinfo();

    return getTotalHeap() - m.uordblks;
}

#define SPI_PORT spi0
#define PIN_MISO 16
#define SD_CS 21
#define PIN_SCK 18
#define PIN_MOSI 19

#ifndef initPins
#define initPins
bool pinsInitialized = false;
#endif

// uint8_t readBuffer[512]; // Buffer to hold the read block

unsigned long totalBlocks = 0;

#define blockOffset 100
#define endOfFile 255
#define endOfName 254
#define deletedFile 253
#define endOfBlock 252

#define SDInitSpeed 40 * 1000          // 40 khz
#define SDNormalSpeed 10 * 1000 * 1000 // 10mhz

typedef struct
{
    char *name;
    uint8_t nameLength;
    uint32_t startBlock;
    uint32_t endBlock;
} File;

File *FileInit(char *name, uint8_t nameLength, uint32_t startBlock, uint32_t endBlock)
{
    File *file = (File *)malloc(sizeof(File));

    file->name = (char *)malloc(sizeof(char) * nameLength + 1);
    for (int i = 0; i < nameLength; i++)
    {
        file->name[i] = name[i];
    }
    file->name[nameLength] = '\0';
    file->nameLength = nameLength;
    file->startBlock = startBlock;

    file->endBlock = endBlock;
    return file;
}
typedef struct
{
    uint8_t data[512];
} Block;

File *NULL_FILE;

void print(const char *message)
{
    printf("%s\n", message);
}

// Takes for bytes and combines them into a uint32
uint32_t CombineBytes(uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

// Takes a uint32 and turns them into 4 bytes
uint8_t *ExtractBytes(uint32_t value)
{
    uint8_t *bytes = (uint8_t *)malloc(4 * sizeof(uint8_t));
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    return bytes;
}
// NOTE: THIS DOES NOT FREE() THE ORIGINAL STRING
char *NullTerminateString(char *string, uint8_t stringLength)
{
    char *newString = (char *)malloc(stringLength * sizeof(char));
    for (int i = 0; i < stringLength; i++)
    {
        newString[i] = string[i];
    }
    newString[stringLength] = '\0';
    return newString;
}

void Flush()
{
    gpio_put(SD_CS, 1);
    sleep_us(5);
    spi_write_blocking(SPI_PORT, (uint8_t[]){0xFF}, 1); // Extra clock
}

// Setup SD card at 400 khz
void InitializeSDPins()
{

    spi_init(SPI_PORT, SDInitSpeed);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // CS is active low
    gpio_init(SD_CS);
    gpio_set_dir(SD_CS, GPIO_OUT);
    gpio_put(SD_CS, 1);

    spi_set_format(SPI_PORT, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    pinsInitialized = true;
}

// Retrieves the total blocks on the SD card
int GetTotalBlocks()
{
    uint8_t cardData[16];
    uint8_t response = 0;

    Flush();

    gpio_put(SD_CS, 0);

    // Get total block count with cmd 9

    uint8_t cmd9[] = {0x40 | 9, 0x00, 0x00, 0x00, 0x00, 0xFF}; // Read CSD register

    spi_write_blocking(SPI_PORT, cmd9, 6);

    do
    {
        // spi_write_blocking(SPI_PORT, (uint8_t[]){0xFF}, 1);
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
    } while ((response & 0x80) != 0); // Wait for 0x00
    printf("Cmd 9A response: %u\n", response);

    do
    {
        // spi_write_blocking(SPI_PORT, (uint8_t[]){0xFF}, 1);
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
        printf("cmd9 %u", response);
    } while (response != 0xFE); // Wait for 0x00
    printf("Cmd 9B response: %u\n", response);

    spi_read_blocking(SPI_PORT, 0xFF, cardData, 16); // Read the CID register

    gpio_put(SD_CS, 1); // Deselect the card

    uint8_t csdType = (cardData[0] >> 6) & 0x03;

    if (csdType == 0)
    {
        // v1.0
        printf("CSD Version 1.0 is not supported\n");
    }
    else if (csdType == 1)
    {
        // v2.0
        uint32_t c_size = ((cardData[7] & 0x3F) << 16) | (cardData[8] << 8) | cardData[9];
        uint32_t blocks = (c_size + 1) * 1024; // 512-byte blocks

        printf("Total blocks (SDHC/SDXC): %lu\n", blocks);
        return blocks;
    }
    else
    {
        printf("Unsupported CSD version\n");
    }

    return 0;
}

Block ReadBlock(uint32_t blockNumber)
{
    if (blockNumber >= totalBlocks)
    {
        print("Block number out of range.\n");
        Block out;
        return out; // Return NULL if block number is out of range
    }

    printf("block num %d\n", blockNumber);

    uint8_t token = 0xFF;

    gpio_put(SD_CS, 1);
    spi_write_blocking(SPI_PORT, &token, 1);
    spi_write_blocking(SPI_PORT, &token, 1);
    gpio_put(SD_CS, 0);
    for (int i = 0; i < 10; i++)
        spi_write_blocking(SPI_PORT, &token, 1);

    uint8_t response = 0;

    uint8_t cmd17[] = {// read block
                       0x40 | 17,
                       (blockNumber >> 24) & 0xFF,
                       (blockNumber >> 16) & 0xFF,
                       (blockNumber >> 8) & 0xFF,
                       blockNumber & 0xFF, 0xFF};

    print("cmd 17");
    spi_write_blocking(SPI_PORT, cmd17, 6);

    print("read");

    do
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
    } while (response == 0xFF);

    printf("response: %d\n", response);

    print("waiting for 0xFE");

    do
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);

    } while (response != 0xFE);

    print("got 0xFE");

    uint8_t readBuffer[512];
    spi_read_blocking(SPI_PORT, 0xFF, readBuffer, 512); // Read the block

    uint8_t crc[2];
    spi_read_blocking(SPI_PORT, 0xFF, crc, 2);

    spi_write_blocking(SPI_PORT, &token, 1);
    gpio_put(SD_CS, 1);
    spi_write_blocking(SPI_PORT, &token, 1);

    Block out;
    memcpy(out.data, readBuffer, 512);

    return out;

    /*print("Cmd17");
    do
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
    } while (response != 0x00); // Wait for 0x00

    print("got 0x00");

    do
    {
        uint8_t x = 0xFF;
        spi_write_read_blocking(SPI_PORT, &x, &response, 1);
        printf("wai start: %d\n", response);
    } while (response != 0xFE); // Wait for Block start token

    printf("Cmd 17b response: %u\n", response);

    uint8_t *readBuffer = (uint8_t *)malloc(sizeof(uint8_t) * 512);

    printf("created read buffer");

    for (int i = 0; i < 512; i++)
    {
        readBuffer[i] = 0; // Initialize the buffer
    }
    spi_read_blocking(SPI_PORT, 0xFF, readBuffer, 512); // Read the block

    uint8_t crc[2];
    spi_read_blocking(SPI_PORT, 0xFF, crc, 2); // cycle 2 crcs
    gpio_put(SD_CS, 1);                        // Deselect the card
    Flush();

    print("read block done");

    return readBuffer; // Return the read block*/
}

void WriteBlock(uint32_t blockNumber, uint8_t *data)
{
    if (blockNumber >= totalBlocks)
    {
        print("Block number out of range.\n");
        return; // Return NULL if block number is out of range
    }

    print("write");

    uint8_t token = 0xFF;

    spi_write_blocking(SPI_PORT, &token, 1);
    gpio_put(SD_CS, 0);
    spi_write_blocking(SPI_PORT, &token, 1);

    uint8_t response = 0;

    uint8_t cmd24[] = {// write block
                       0x40 | 24,
                       (uint8_t)(blockNumber >> 24),
                       (uint8_t)(blockNumber >> 16),
                       (uint8_t)(blockNumber >> 8),
                       (uint8_t)(blockNumber),
                       0xFF};

    print("cmd24");
    spi_write_blocking(SPI_PORT, cmd24, 6);

    print("read response");
    do
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
    } while (response != 0x00);

    print("send start token");
    uint8_t startToken = 0xFE;
    spi_write_blocking(SPI_PORT, &startToken, 1);

    print("send data");
    spi_write_blocking(SPI_PORT, data, 512);

    print("wait response");
    do
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);

    } while (response == 0xFF || response & 0x1F != 0x05);

    print("success");

    while (1)
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
        if (response != 0x00)
            break;
    }
    print("data written");

    spi_write_blocking(SPI_PORT, &token, 1);
    gpio_put(SD_CS, 1);
    spi_write_blocking(SPI_PORT, &token, 1);
}

void SetupTable(bool forceClear)
{

    Block data = ReadBlock(0);

    if (data.data[0] == endOfFile && !forceClear)
    {
        return;
    }
    print("setting up table");

    uint8_t buffer[512];

    for (int i = 0; i < 512; i++)
    {
        buffer[i] = 0;
    }

    // clear 15 blocks for good measure
    for (int b = 1; b < 15; b++)
    {
        WriteBlock(b, buffer);
    }

    buffer[0] = endOfFile;
    buffer[9] = endOfFile;

    WriteBlock(0, buffer);
}

int InitializeSDCard()
{
    if (!pinsInitialized)
    {
        InitializeSDPins();
    }

    NULL_FILE = FileInit("", 0, 0, 0);

    ////////////////////////////////// step 1 ///////////////////////////////

    print("step1A\n");
    // Send dummy clocks at least 74
    gpio_put(SD_CS, 1); // Deselect the card
    for (int i = 0; i < 10; i++)
    {
        uint8_t dummy = 0xFF;
        spi_write_blocking(SPI_PORT, &dummy, 1);
    }

    print("step1B\n");

    //////////////////////////////////// step 2 ///////////////////////////////

    print("step2A\n");
    uint8_t cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95}; // Idle state with CRC

    gpio_put(SD_CS, 0); // Enable card
    spi_write_blocking(SPI_PORT, cmd0, 6);

    uint8_t response;
    for (int i = 0; i < 8; i++)
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
        if ((response & 0x80) == 0)
        {
            break; // Response received
        }
    }
    printf("Cmd 0 response: %u\n", response);
    gpio_put(SD_CS, 1);
    print("step2B\n");

    ////////////////////////////////// step 3 ///////////////////////////////

    print("step3A\n");
    uint8_t cmd8[] = {0x40 | 8, 0x00, 0x00, 0x01, 0xAA, 0x87}; // Check card specs and CRC

    gpio_put(SD_CS, 0);

    spi_write_blocking(SPI_PORT, cmd8, 6);

    int timeout = 0;

    do
    {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
        timeout++;
    } while (timeout < 8 && response != 0x01);

    if (response != 0x01)
    {
        return 1;
    }

    uint8_t response7[4];
    spi_read_blocking(spi0, 0xFF, response7, 4);
    for (int i = 0; i < 4; i++)
    {
        printf("R8B[%u]: %u\n", i, response7[i]);
    }

    gpio_put(SD_CS, 1);
    print("step3B\n");
    sleep_ms(100); // Wait for a short time before the next command
    /////////////////////////////////// step 4 ///////////////////////////////
    print("step4A\n");

    uint8_t cmd55[] = {0x40 | 55, 0x00, 0x00, 0x00, 0x00, 0xFF}; // Set mode to application cmd and CRC

    timeout = 0;
    do
    {
        gpio_put(SD_CS, 0);

        spi_write_blocking(SPI_PORT, cmd55, 6);
        for (int i = 0; i < 8; i++)
        {
            spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
            if (response == 0x01)
            {
                break; // Response received
            }
        }

        // printf("55R: %u\n", response);

        if (response != 0x01)
        {
            print("Failed to receive response for CMD55.\n");
            continue; // Retry if response is not 0x01
        }

        uint8_t acmd41[] = {0x69, 0x40, 0x00, 0x00, 0x00, 0xFF}; // Initialize card

        spi_write_blocking(SPI_PORT, acmd41, 6);

        for (int i = 0; i < 128; i++)
        {
            spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
            if (response == 0x00)
            {
                break; // Response received
            }
        }

        // printf("A41R: %u\n", response);

        if (response == 0x00)
        {
            print("Card initialized successfully.\n");
            // Deselect card
            break; // Exit loop if card is initialized
        }

        gpio_put(SD_CS, 1);                      // CS HIGH after both
        uint8_t dummy = 0xFF;                    // Send a dummy byte
        spi_write_blocking(SPI_PORT, &dummy, 1); // Send 0xFF

        timeout++;

    } while (timeout < 1000);
    gpio_put(SD_CS, 1);
    if (response != 0x00)
    {
        print("Card initialization failed.\n");
        return 2; // Return error code if initialization fails
    }

    sleep_ms(100);

    print("step4B\n");

    ////////////////////////////////// step 5 ///////////////////////////////
    print("step5A\n");
    timeout = 150;
    uint8_t ocr[4];
    do
    {

        uint8_t cmd58[] = {0x40 | 58, 0x00, 0x00, 0x00, 0x00, 0x00}; // Read OCR

        gpio_put(SD_CS, 0);
        spi_write_blocking(SPI_PORT, cmd58, 6);
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);

        spi_read_blocking(SPI_PORT, 0xFF, ocr, 4);
        gpio_put(SD_CS, 1);

        timeout--;

    } while ((ocr[0] & 0b10000000) == 1 && timeout > 0);
    print("step5B\n");

    for (int i = 0; i < 4; i++)
    {
        printf("OCR[%u]: %u\n", i, ocr[i]);
    }

    if (timeout == 0)
    {
        print("SD Card initialization failed.\n");
        return 3;
    }

    // Additional SD card initialization code can be added here
    print("SD Card initialized.\n");

    totalBlocks = GetTotalBlocks();

    SetupTable(false);

    return 0;
}

bool Strcmp(char *str1, char *str2)
{
    int index = 0;
    while (str1[index] != '\0' && str2[index] != '\0')
    {
        if (str1[index] != str2[index])
        {
            return false;
        }
        index++;
    }

    if ((str1[index] != '\0') ^ (str2[index] != '\0'))
    {
        return false; // If one string is longer than the other
    }

    return true;
}

File *GetFile(char *searchFileName)
{
    printf("GetFile current heap %u/%u\n", getFreeHeap(), getTotalHeap());
    printf("Get File: %s\n", searchFileName);
    int fileIndex = 9;
    int block = 0;
    int currentFile = 0;
    Block buffer = ReadBlock(0);

    printf("read block");

    uint32_t files = CombineBytes(buffer.data + 5);

    printf("get file setup %u\n", files);

    while (currentFile < files)
    {
        print("search");
        fileIndex++;
        char *fileName = (char *)malloc((buffer.data[fileIndex] + 1) * sizeof(char));
        fileName[buffer.data[fileIndex]] = '\0'; // Null-terminate the string
        for (int i = 0; i < buffer.data[fileIndex]; i++)
        {
            fileName[i] = buffer.data[fileIndex + 1 + i];
        }

        if (buffer.data[fileIndex - 1] != deletedFile && Strcmp(searchFileName, fileName))
        {
            print("found match");
            File *file = FileInit(fileName, (buffer.data[fileIndex] + 1), CombineBytes(buffer.data + fileIndex + 1 + buffer.data[fileIndex] + 1), CombineBytes(buffer.data + fileIndex + 1 + buffer.data[fileIndex] + 1 + 4) + 1);
            free(fileName);
            print("returning");
            return file;
        }
        else
        {
            fileIndex += buffer.data[fileIndex] + 10; // Skip the file name length + 10
        }

        free(fileName);

        if (buffer.data[fileIndex] == endOfBlock)
        {
            block++;
            fileIndex = 0;
            memcpy(buffer.data, ReadBlock(block).data, 512);
        }

        currentFile++;
    }
    print("file not found");

    return NULL_FILE; // Return an empty file if not found
}

uint8_t *ReadFile(File *file)
{
    uint8_t blocksOccupied = file->startBlock - file->endBlock + 1;
    uint8_t *data = (uint8_t *)malloc(blocksOccupied * 512 * sizeof(uint8_t));

    for (int i = 0; i < blocksOccupied * 512; i++)
    {
        data[i] = 0;
    }

    for (int i = 0; i < blocksOccupied; i++)
    {
        printf("Reading block %u\n", file->startBlock - i);
        Block blockData = ReadBlock(file->startBlock - i);
        if (blockData.data == NULL)
        {
            free(data);
            return NULL;
        }
        for (int j = 0; j < 512; j++)
        {
            data[i * 512 + j] = blockData.data[j];
        }
    }

    return data; // Return the data read from the file
}

uint8_t *ReadFileUntil(File *file, char until)
{
    uint8_t blocksOccupied = file->startBlock - file->endBlock + 1;
    uint8_t *data = (uint8_t *)malloc(blocksOccupied * 512 * sizeof(uint8_t));

    for (int i = 0; i < blocksOccupied * 512; i++)
    {
        data[i] = 0;
    }

    for (int i = 0; i < blocksOccupied; i++)
    {
        printf("Reading block %u\n", file->startBlock - i);
        Block blockData = ReadBlock(file->startBlock - i);

        for (int j = 0; j < 512; j++)
        {
            data[i * 512 + j] = blockData.data[j];
            if (blockData.data[j] == until)
            {
                return data;
            }
        }
    }

    return data; // Return the data read from the file
}

uint8_t *ReadFileUntilLimited(File *file, char until, uint8_t maxBlocks)
{
    uint8_t blocksOccupied = file->startBlock - file->endBlock + 1;
    if (blocksOccupied > maxBlocks)
    {
        blocksOccupied = maxBlocks;
    }
    uint8_t *data = (uint8_t *)malloc(blocksOccupied * 512 * sizeof(uint8_t));

    for (int i = 0; i < blocksOccupied * 512; i++)
    {
        data[i] = 0;
    }

    for (int i = 0; i < blocksOccupied; i++)
    {
        printf("Reading block %u\n", file->startBlock - i);
        Block blockData = ReadBlock(file->startBlock - i);

        for (int j = 0; j < 512; j++)
        {
            data[i * 512 + j] = blockData.data[j];
            if (blockData.data[j] == until)
            {
                return data;
            }
        }
    }

    return data; // Return the data read from the file
}

void WriteFile(File *file, char *data, uint32_t length)
{
    uint8_t blocksOccupied = file->startBlock - file->endBlock + 1;

    if (length >= blocksOccupied * 512)
    {
        print("Data size exceeds allocated blocks.\n");
        return;
    }
    uint8_t buffer[512];
    for (int b = 0; b < blocksOccupied; b++)
    {

        for (int i = 0; i < 512; i++)
        {
            if (i + b * 512 < length)
            {
                printf("Write: %c\n", (data[i + b * 512]));
                buffer[i] = (uint8_t)(data[i + b * 512]);
            }
            else
            {
                buffer[i] = 0;
            }
        }
        printf("write file block: %d\n", file->startBlock - b);
        WriteBlock(file->startBlock - b, buffer);
    }
}

File *CreateFile(char *filename, uint8_t fileNameLen, uint8_t requestBlocks)
{
    print("Create File");
    printf("Creating file with name len of %u\n", strlen(filename));
    File *checkExist = GetFile(filename);
    print("Check if exist");
    if (checkExist->startBlock != 0)
    {
        print("file exists");
        free(checkExist);
        return NULL_FILE;
    }

    Block buffer = ReadBlock(0);

    uint32_t blocksUsed = CombineBytes(buffer.data + 1);
    uint32_t files = CombineBytes(buffer.data + 5);

    uint8_t *blockUsedExtracted = ExtractBytes(blocksUsed + requestBlocks);
    uint8_t *filesExtracted = ExtractBytes(files + 1);
    for (int i = 0; i < 4; i++)
    {
        buffer.data[i + 1] = blockUsedExtracted[i];
    }
    for (int i = 0; i < 4; i++)
    {
        buffer.data[i + 5] = filesExtracted[i];
    }
    free(blockUsedExtracted);
    free(filesExtracted);

    buffer.data[0] = endOfFile;

    if (buffer.data[9] != deletedFile)
        buffer.data[9] = endOfFile;

    print("TableSpec updated.\n");
    WriteBlock(0, buffer.data);

    int fileIndex = 9;
    int currentFile = 0;
    int writeBlock = 0;

    while (currentFile < files)
    {
        printf("Current file: %u, File index: %u, Write block: %u\n", currentFile, fileIndex, writeBlock);
        fileIndex++;
        fileIndex += buffer.data[fileIndex] + 10; // Skip the file name length+ 10
        if (buffer.data[fileIndex] == endOfBlock)
        {
            writeBlock++;
            fileIndex = 0;
            memcpy(buffer.data, ReadBlock(writeBlock).data, 512);
        }

        currentFile++;
    }
    fileIndex++;

    if (fileIndex > 450)
    {
        buffer.data[fileIndex - 1] = endOfBlock;
        WriteBlock(writeBlock++, buffer.data);
        fileIndex = 0;
        // If there may not be enough space left in this block, write to the next block
    }

    printf("Free space at index: %u, block: %u\n", fileIndex, writeBlock);

    uint32_t startBlock = totalBlocks - 100 - blocksUsed - 1;
    uint32_t endBlock = startBlock - requestBlocks;

    printf("Start: %u, End: %u\n", startBlock, endBlock);

    buffer.data[fileIndex] = fileNameLen;
    fileIndex++;

    for (int i = 0; i < fileNameLen; i++)
    {
        buffer.data[fileIndex + i] = filename[i];
    }
    buffer.data[fileIndex + fileNameLen] = endOfName; // Mark the end of the name

    uint8_t *startExtracted = ExtractBytes(startBlock);
    uint8_t *endExtracted = ExtractBytes(endBlock);

    for (int i = 0; i < 4; i++)
    {
        buffer.data[fileIndex + fileNameLen + 1 + i] = startExtracted[i];
    }
    for (int i = 0; i < 4; i++)
    {
        buffer.data[fileIndex + fileNameLen + 1 + i + 4] = endExtracted[i];
    }
    free(startExtracted);
    free(endExtracted);

    buffer.data[fileIndex + fileNameLen + 1 + 4 + 4] = endOfFile; // Mark the end of the file

    WriteBlock(writeBlock, buffer.data); // Write the updated buffer back to the block

    uint8_t clearData[512];
    for (int i = 0; i < 512; i++)
    {
        clearData[i] = 0;
    }
    for (int i = startBlock; i > endBlock; i--)
    {
        WriteBlock(i, clearData);
    }
    return FileInit(filename, fileNameLen, startBlock, endBlock);
}

void DeleteFile(char *searchFileName)
{
    int fileIndex = 9;
    int block = 0;
    int currentFile = 0;
    Block buffer = ReadBlock(0);

    uint32_t files = CombineBytes(buffer.data + 5);

    while (currentFile < files)
    {

        fileIndex++;
        char *fileName = (char *)malloc((buffer.data[fileIndex] + 1) * sizeof(char));
        fileName[buffer.data[fileIndex]] = '\0'; // Null-terminate the string
        for (int i = 0; i < buffer.data[fileIndex]; i++)
        {
            fileName[i] = buffer.data[fileIndex + 1 + i];
        }

        if (Strcmp(searchFileName, fileName) && buffer.data[fileIndex - 1] != deletedFile)
        {
            printf("deleting b%u, i%u", block, fileIndex - 1);
            buffer.data[fileIndex - 1] = deletedFile;
            WriteBlock(block, buffer.data);
            free(fileName);
            return;
        }
        else
        {
            fileIndex += buffer.data[fileIndex] + 10; // Skip the file name length + 10
        }

        free(fileName);

        if (buffer.data[fileIndex] == endOfBlock)
        {
            block++;
            fileIndex = 0;
            memcpy(buffer.data, ReadBlock(block).data, 512);
        }

        currentFile++;
    }
}

File *GetFileByIndex(int index)
{
    int fileIndex = 9;
    int block = 0;
    int currentFile = 0;
    Block buffer = ReadBlock(0);

    uint32_t files = CombineBytes(buffer.data + 5);

    printf("Files: %u\n", files);

    if (index >= files)
    {
        print("file out of range");
        return NULL_FILE; // Return an empty file if not found
    }

    while (currentFile < files)
    {

        fileIndex++;
        char *fileName = (char *)malloc((buffer.data[fileIndex] + 1) * sizeof(char));
        fileName[buffer.data[fileIndex]] = '\0'; // Null-terminate the string
        for (int i = 0; i < buffer.data[fileIndex]; i++)
        {
            fileName[i] = buffer.data[fileIndex + 1 + i];
        }

        if (buffer.data[fileIndex - 1] != deletedFile)
        {
            if (currentFile == index)
            {
                printf("found file at: %u\n", fileIndex);
                File *file = FileInit(fileName, (buffer.data[fileIndex] + 1), CombineBytes(buffer.data + fileIndex + 1 + buffer.data[fileIndex] + 1), CombineBytes(buffer.data + fileIndex + 1 + buffer.data[fileIndex] + 1 + 4) + 1);
                free(fileName);
                return file;
            }
            currentFile++;
        }

        fileIndex += buffer.data[fileIndex] + 10;

        free(fileName);

        if (buffer.data[fileIndex] == endOfBlock)
        {
            block++;
            fileIndex = 0;
            memcpy(buffer.data, ReadBlock(block).data, 512);
        }
    }
    return NULL_FILE;
}

int GetFileCount()
{
    int fileIndex = 9;
    int block = 0;
    int currentFile = 0;
    int activeFiles = 0;
    print("SD GetFileCount");
    Block buffer = ReadBlock(0);
    print("SD GetFileCount: read block");

    uint32_t files = CombineBytes(buffer.data + 5);
    printf("SD GetFileCount: got file count %u\n", files);

    while (currentFile < files)
    {

        fileIndex++;
        char *fileName = (char *)malloc((buffer.data[fileIndex] + 1) * sizeof(char));
        print("SD GetFileCount: malloc name");
        fileName[buffer.data[fileIndex]] = '\0'; // Null-terminate the string
        for (int i = 0; i < buffer.data[fileIndex]; i++)
        {
            fileName[i] = buffer.data[fileIndex + 1 + i];
        }

        currentFile++;
        if (buffer.data[fileIndex - 1] != deletedFile)
        {

            activeFiles++;
        }

        fileIndex += buffer.data[fileIndex] + 10;

        free(fileName);
        print("SD GetFileCount: free");

        if (buffer.data[fileIndex] == endOfBlock)
        {
            block++;
            fileIndex = 0;
            memcpy(buffer.data, ReadBlock(block).data, 512);
        }
    }
    print("SD GetFileCount: end");
    return activeFiles;
}

#endif