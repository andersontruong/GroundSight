#include "FS.h" 
#include "SD_MMC.h"

//List dir in SD card
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

//Create a dir in SD card
void createDir(fs::FS &fs, const char * path);

//delete a dir in SD card
void removeDir(fs::FS &fs, const char * path);

//Read a file in SD card
void readFile(fs::FS &fs, const char * path);

//Write a file in SD card
void writeFile(fs::FS &fs, const char * path, const char * message);

//Append to the end of file in SD card
void appendFile(fs::FS &fs, const char * path, const char * message);

//Rename a file in SD card
void renameFile(fs::FS &fs, const char * path1, const char * path2);

//Delete a file in SD card
void deleteFile(fs::FS &fs, const char * path);

//Test read and write speed using test.txt file
void testFileIO(fs::FS &fs, const char * path);