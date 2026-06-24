#include "DebugUART.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <fstream>

bool DebugUART::s_initialized = false;
bool DebugUART::s_fileEnabled = true;
size_t DebugUART::s_maxFileBytes = 25000;
std::string DebugUART::s_buffer;
std::mutex DebugUART::s_mutex;

void DebugUART::init(bool enableFile, size_t maxFileBytes) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_fileEnabled = enableFile;
    s_maxFileBytes = maxFileBytes;
    s_buffer.clear();
    s_buffer.reserve(maxFileBytes);
    s_initialized = true;
}

void DebugUART::enableFile(bool enabled) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_fileEnabled = enabled;
}

void DebugUART::log(const char* format, ...) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_initialized) {
        return;
    }

    char temp[1024];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);
    if (len <= 0) {
        return;
    }

    size_t writeLen = static_cast<size_t>(len);
    if (writeLen >= sizeof(temp)) {
        writeLen = sizeof(temp) - 1;
        temp[writeLen] = '\0';
    }

    appendText(temp, writeLen);
}

void DebugUART::logRaw(const char* text) {
    if (text == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_initialized) {
        return;
    }

    appendText(text, std::strlen(text));
}

void DebugUART::appendText(const char* data, size_t len) {
    if (len == 0) {
        return;
    }

    std::fwrite(data, 1, len, stdout);
    std::fflush(stdout);

    if (!s_fileEnabled || s_maxFileBytes == 0) {
        return;
    }

    size_t remaining = (s_buffer.size() >= s_maxFileBytes) ? 0 : (s_maxFileBytes - s_buffer.size());
    if (remaining == 0) {
        return;
    }

    size_t appendLen = len;
    if (appendLen > remaining) {
        appendLen = remaining;
    }
    s_buffer.append(data, appendLen);
}

void DebugUART::saveToFile(const char* path) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_fileEnabled || s_buffer.empty()) {
        return;
    }

    std::ofstream out(path, std::ofstream::out | std::ofstream::trunc);
    if (!out.is_open()) {
        return;
    }

    out.write(s_buffer.data(), s_buffer.size());
    out.close();
}
