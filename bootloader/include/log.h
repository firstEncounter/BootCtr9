#pragma once

#include "common.h"


int initLog();
void closeLogFile();
void debug(const char *format, ...);
void panic(const char *format, ...);
void shutdown();