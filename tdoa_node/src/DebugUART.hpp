#pragma once

#include <cstdarg>
#include <cstddef>
#include <mutex>
#include <string>

class DebugUART {
public:
	static void init(bool enableFile = true, size_t maxFileBytes = 25000);
	static void enableFile(bool enabled);
	static void log(const char* format, ...);
	static void logRaw(const char* text);
	static void saveToFile(const char* path = "debug.txt");

private:
	static void appendText(const char* data, size_t len);

	static bool s_initialized;
	static bool s_fileEnabled;
	static size_t s_maxFileBytes;
	static std::string s_buffer;
	static std::mutex s_mutex;
};
