#pragma once
#include <iostream>
namespace serialLogger{
void log_example();
void header();
void blank_lines(int);
void ruler();
void logLongFloat(const float *dataPtr, const int, const char []);
void logFloat(const float *dataPtr, const int, const char [], const char[]);
void logFloat(const float *dataPtr, const int, const char []);
void logFloat(const float *dataPtr, const int, const int, const char []);
void logFloat(const float *array[], const int, const int, const char []);
void logInt64(const int64_t *data, const char header[]);
void logInt(const int  *data, const char header[]);
void logUInt8(const uint8_t  *data, const char header[]);
void logUInt16(const uint16_t  *data, const char header[]);
};
