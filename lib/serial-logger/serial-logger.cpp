#include "serial-logger.h"
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace serialLogger{

void log_example()
{
    std::cout << "Hey!" << std::endl;
    for (int i = 0; i < 100000; i++)
    {
        std::cout << i << ","<< i+2 << std::endl;
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void header()
{
    blank_lines(3);
    ruler();
    std::cout << "Starting..." << std::endl;
    ruler();
}

void blank_lines(int line_number = 1)
{
    for (int i = 0; i < line_number; i++)
    {
        std::cout << "\n";
    }
}

void ruler()
{
    std::cout << "--------------------" << std::endl;
}

void logLongFloat(const float *dataPtr, const int lenData, const char header[]){
    std::cout.precision(14);
    std::cout << header << " ";
    for (int j = 0; j < lenData; j++){
        std::cout << *(dataPtr + j) << " ";
    }
    std::cout << std::endl;
};

void logFloat(const float *dataPtr, const char header[]){
    std::cout << header << " " << *(dataPtr) << std::endl;
};

void logFloat(const float *dataPtr, const int lenData, const int K, const char header[]){
    std::cout << header << " ";
    for (int j = 0; j < lenData; j++){
        std::cout << *(dataPtr + j)*K << " ";
    }
    std::cout << std::endl;
};

void logFloat(const float *dataPtr, const int lenData,
            const char header[], const char separator[]){
    std::cout << header << " ";
    for (int j = 0; j < lenData - 1; j++){
        std::cout << *(dataPtr + j) << separator;
    }
    std::cout << *(dataPtr + lenData - 1) << std::endl;
};

void logFloat(const float *dataPtr, const int lenData, const char header[]){
    std::cout << header << " ";
    for (int j = 0; j < lenData; j++){
        std::cout << *(dataPtr + j) << " ";
    }
    std::cout << std::endl;
};

void logFloat(const float *dataPtrArray[], const int lenArray, const int lenData, const char header[]){
    std::cout << header << " ";
    for (int i = 0; i < lenArray; i++){
        for (int j = 0; j < lenData; j++){
            std::cout << *(dataPtrArray[i]  + j) << " ";
        }
    }
    std::cout << std::endl;
};

void logUInt8(const uint8_t *data, const char header[]){
    std::cout << header << " ";
    std::cout << (int)(*data) << " ";
    std::cout << std::endl;
};

void logUInt16(const uint16_t *data, const char header[]){
    std::cout << header << " ";
    std::cout << (int)(*data) << " ";
    std::cout << std::endl;
};

void logInt(const int *data, const char header[]){
    std::cout << header << " ";
    std::cout << (int)(*data) << " ";
    std::cout << std::endl;
};

void logInt64(const int64_t *data, const char header[]){
    std::cout << header << " ";
    std::cout << (int64_t)(*data) << " ";
    std::cout << std::endl;
};

};
