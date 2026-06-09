// File: preprocessing_ts.c
// Implements anomaly detection edge algorithms.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_ts.h"

typedef struct {
    double features[MAX_TS_FEATURES];
    int label; 
} TSRecord;

int parse_ts_csv(const char* filepath, TSRecord* records) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("[ERROR] Could not open time-series dataset");
        return -1;
    }

    char line[2048];
    int row_count = 0;
    int normal_count = 0;
    int anomaly_count = 0;

    if (!fgets(line, sizeof(line), file)) return 0;

    while (fgets(line, sizeof(line), file)) {
        if (row_count >= MAX_ROWS) break; 

        char* token = strtok(line, ",");
        int col_idx = 0;
        
        while (token != NULL) {
            if (col_idx == 0) records[row_count].features[0] = atof(token);
            if (col_idx == 1) records[row_count].features[1] = atof(token);
            if (col_idx == 2) records[row_count].features[2] = atof(token);
            if (col_idx == 3) records[row_count].features[3] = atof(token);
            if (col_idx == 4) records[row_count].label = atoi(token);
            
            token = strtok(NULL, ",");
            col_idx++;
        }
        
        if (records[row_count].label == 1) {
            anomaly_count++;
        } else {
            normal_count++;
        }

        row_count++;
    }

    fclose(file);
    printf("[INFO] Parse Complete: Loaded %d Normal and %d Synthetic Attacks.\n", normal_count, anomaly_count);
    return row_count;
}