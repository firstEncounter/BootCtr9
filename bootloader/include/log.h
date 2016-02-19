#pragma once

#include "common.h"


int initLog();
void closeLogFile();
void debug(const char *msg);
void panic(const char *msg);
void shutdown();