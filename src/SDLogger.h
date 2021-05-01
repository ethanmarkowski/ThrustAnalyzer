// SDLogger.h

#ifndef _SDLogger_h
#define _SDLogger_h

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SDLogger
{
private:
	const uint8_t _csPin;
	bool _isEnabled;
	String _filename;
	File _logFile;

public:
	SDLogger(const uint8_t &csPin);
	bool CheckCard() const;
	void Enable();
	void Disable();
	bool IsEnabled() const;
	void CreateNewFile();
	String GetFilename() const;
	void Log(const String *values, const uint8_t &numValues);
	void Save();
};

#endif