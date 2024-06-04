#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define FIND(K, BUF) (!strncmp(K, BUF, strlen(K)))
#define MAX_LINE 200

int load_cfg();
