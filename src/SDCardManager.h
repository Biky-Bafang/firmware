#ifndef SDCARDMANAGER_H // Header guard start
#define SDCARDMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SD_MMC.h>
#include <FS.h>
#include <RTOS.h>

#define CLK 4
#define CMD 6
#define DATA0 5
#define CD 7

// Class definition
class SDCardManager
{
public:
	SDCardManager(); // Constructor declaration
	void init(JsonDocument *settings);
	void restart(JsonDocument *settings);
	void writeData(String data);
	void readData();
	void deleteData();
	void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
	void createDir(fs::FS &fs, const char *path);
	void removeDir(fs::FS &fs, const char *path);
	void readFile(fs::FS &fs, const char *path);
	void writeFile(fs::FS &fs, const char *path, const char *message);
	void appendFile(fs::FS &fs, const char *path, const char *message);
	void renameFile(fs::FS &fs, const char *path1, const char *path2);
	void deleteFile(fs::FS &fs, const char *path);
	void testFileIO(fs::FS &fs, const char *path);

private:
	static void loop(void *parameter);
	struct sdCardParams
	{
		JsonDocument &settings;
		SDCardManager *instance; // Add instance pointer for accessing non-static members
	};
	static TaskHandle_t taskHandle;
};

#endif // Header guard end
