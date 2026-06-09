#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_flow.h"

typedef struct {
    double features[MAX_FLOW_FEATURES];
    int label; // 0 for normal, 1 for anomaly
} FlowRecord;



int parse_flow_csv(const char* filepath, FlowRecord* records) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("[ERROR] Could not open dataset");
        return -1;
    }

    char line[2048];
    int row_count = 0;
    
    // Balanced Loading Targets
    int normal_count = 0;
    int anomaly_count = 0;
    int target_normal = 2500; 
    int target_anomaly = MAX_ROWS - target_normal; // 500

    // Skip header
    if (!fgets(line, sizeof(line), file)) return 0;

    while (fgets(line, sizeof(line), file)) {
        if (row_count >= MAX_ROWS) {
            printf("[INFO] Reached balanced dataset limit (%d rows).\n", MAX_ROWS);
            break; 
        }

        // Temporary storage to evaluate the row before committing to our bounded memory
        double temp_features[MAX_FLOW_FEATURES] = {0};
        int temp_label = 0;

        char* token = strtok(line, ",");
        int col_idx = 0;
        
        while (token != NULL) {
            if (col_idx == 0) temp_features[0] = atof(token);
            if (col_idx == 1) temp_features[1] = atof(token);
            if (col_idx == 2) temp_features[2] = atof(token);
            if (col_idx == 3) temp_features[3] = atof(token);
            if (col_idx == 4) temp_label = atoi(token);
            
            token = strtok(NULL, ",");
            col_idx++;
        }
        
        // BALANCED LOADING LOGIC: Only save if we still need this specific type of traffic
        int should_save = 0;
        if (temp_label == 0 && normal_count < target_normal) {
            normal_count++;
            should_save = 1;
        } else if (temp_label == 1 && anomaly_count < target_anomaly) {
            anomaly_count++;
            should_save = 1;
        }

        // Commit to main array if it met the criteria
        if (should_save) {
            records[row_count].features[0] = temp_features[0];
            records[row_count].features[1] = temp_features[1];
            records[row_count].features[2] = temp_features[2];
            records[row_count].features[3] = temp_features[3];
            records[row_count].label = temp_label;
            
            row_count++;
        }
    }

    fclose(file);
    printf("[INFO] Parse Complete: Loaded %d Normal flows and %d Anomalous flows.\n", normal_count, anomaly_count);
    return row_count;
}