// SDLogger.cpp

#include "SDLogger.h"

SDLogger::SDLogger(const uint8_t &csPin) : _csPin(csPin), _isEnabled(false), _filename("") { pinMode(53, OUTPUT); }

bool SDLogger::CheckCard() const { return SD.begin(); }

void SDLogger::Enable() { _isEnabled = true; }

void SDLogger::Disable() { _isEnabled = false; }

bool SDLogger::GetIsEnabled() const { return _isEnabled; }

void SDLogger::CreateNewFile()
{
	// Find the first unused filename on the card that follows the format of data-<number>.log
	for (uint8_t i = 1; !SD.exists("data-" + String(i) + ".log"); ++i) 
	{
		_filename = "data-" + String(i) + ".log";
	}

	// Create SD file object
	_logFile = SD.open(_filename, FILE_WRITE);
}

String SDLogger::GetFilename() const { return _filename; }

void SDLogger::Log(const String *values, const uint8_t &numValues)
{
	// Construct comma separated log line of headers
	String buffer = values[0];
	for (uint8_t i = 1; i < numValues; ++i)
	{
		buffer.concat("," + values[i]);
	}

	// Print buffer to log file
	_logFile.println(buffer);
}

void SDLogger::Save()
{
	_logFile.close();
}