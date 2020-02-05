  
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define DEBUG_MESSAGE
#ifdef DEBUG_MESSAGE
#define DBG_MSG(format, ...) printf("[DBG]%s: " format, __func__, ##__VA_ARGS__)
#else
#define DBG_MSG(format, ...) do{}while(0)
#endif

#define INF_MSG(format, ...) printf("[INF]%s: " format, __func__, ##__VA_ARGS__)
#define ERR_MSG(format, ...) printf("[ERR]%s: " format, __func__, ##__VA_ARGS__)
