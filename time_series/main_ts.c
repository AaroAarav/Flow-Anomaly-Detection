// File: main_ts.c
// Implements anomaly detection edge algorithms.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "config_ts.h"

typedef struct { double features[MAX_TS_FEATURES]; int label; } TSRecord;
extern int parse_ts_csv(const char* filepath, TSRecord* records);
extern HardwareStats get_hardware_snapshot();

typedef struct { double level; double trend; double seasonal[HW_SEASON_LENGTH]; int season_idx; double residual_mean; double residual_variance; int n_updates; } HoltWintersTS;
extern void init_hw_ts(HoltWintersTS* hw, double initial_val);
extern int evaluate_hw_ts_anomaly(HoltWintersTS* hw, double current_val, double threshold);

typedef struct { void* trees[IFOR_NUM_TREES]; } IsolationForestTS;
extern double score_iforest_ts(IsolationForestTS* forest, double* features);
extern void fit_multi_algos_ts(TSRecord* records, int train_rows);
extern double score_centroid_euclidean_ts(TSRecord* record);
extern double score_centroid_manhattan_ts(TSRecord* record);
extern double score_mahalanobis_ts(TSRecord* record);
extern double score_epsilon_density_ts(TSRecord* records, int target_idx, int num_rows, double epsilon);
extern double score_hypercube_ts(TSRecord* record);

void evaluate_and_log(FILE* csv, const char* algo, double thresh, int tp, int fp, int tn, int fn, int total_test_rows, HardwareStats start, HardwareStats end) {
    double accuracy = (total_test_rows > 0) ? (double)(tp + tn) / total_test_rows : 0.0;
    double precision = (tp + fp > 0) ? (double)tp / (tp + fp) : 0.0;
    double recall = (tp + fn > 0) ? (double)tp / (tp + fn) : 0.0;
    double f1 = (precision + recall > 0) ? 2.0 * (precision * recall) / (precision + recall) : 0.0;
    
    double latency = end.timestamp_ms - start.timestamp_ms;
    long cpu_usage = end.cpu_time_us - start.cpu_time_us;
    long disk_io = end.disk_io_bytes - start.disk_io_bytes;
    long peak_ram = end.ram_kb; 

    fprintf(csv, "%s,%.2f,%d,%d,%d,%d,%.4f,%.4f,%.4f,%.4f,%.2f,%ld,%ld,%ld\n", 
            algo, thresh, tp, fp, tn, fn, accuracy, precision, recall, f1, latency, cpu_usage, peak_ram, disk_io);
            
    printf("[%s] Thresh %8.2f -> F1: %.4f | Latency: %6.2fms | CPU: %8ldus | RAM: %ldKB\n", 
            algo, thresh, f1, latency, cpu_usage, peak_ram);
}

int main(int argc, char *argv[]) {
    char* dataset_path = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1) { if (opt == 'd') dataset_path = optarg; }
    if (!dataset_path) return -1;

    TSRecord* records = (TSRecord*)malloc(sizeof(TSRecord) * MAX_ROWS);
    int loaded_rows = parse_ts_csv(dataset_path, records);
    
    int train_rows = (int)(loaded_rows * 0.8);
    int test_rows = loaded_rows - train_rows;
    printf("[INFO] Train/Test Split: 80/20 (%d Training Burn-in, %d Testing)\n", train_rows, test_rows);
    
    FILE* results_csv = fopen("../results/ts_param_sweep.csv", "w");
    fprintf(results_csv, "Algorithm,Threshold,TP,FP,TN,FN,Accuracy,Precision,Recall,F1_Score,Latency_ms,CPU_us,Peak_RAM_KB,Disk_IO_Bytes\n");
    printf("\n--- Starting Time Series Hardware Sweep ---\n");

    for (double thresh = 1.0; thresh <= 5.0; thresh += 0.5) {
        HoltWintersTS hw;
        if (loaded_rows > 0) init_hw_ts(&hw, records[0].features[1]); 
        int tp = 0, fp = 0, tn = 0, fn = 0;

        HardwareStats start_stats = get_hardware_snapshot();
        for (int i = 0; i < loaded_rows; i++) {
            int is_anomaly = evaluate_hw_ts_anomaly(&hw, records[i].features[1], thresh);
            if (i >= train_rows) {
                int actual = records[i].label;
                if (is_anomaly && actual) tp++; else if (is_anomaly && !actual) fp++;
                else if (!is_anomaly && !actual) tn++; else if (!is_anomaly && actual) fn++;
            }
        }
        HardwareStats end_stats = get_hardware_snapshot();
        evaluate_and_log(results_csv, "Holt-Winters", thresh, tp, fp, tn, fn, test_rows, start_stats, end_stats);
    }

    IsolationForestTS forest;
    memset(&forest, 0, sizeof(IsolationForestTS)); 
    for (double thresh = 0.45; thresh <= 0.70; thresh += 0.05) {
        int tp = 0, fp = 0, tn = 0, fn = 0;
        
        HardwareStats start_stats = get_hardware_snapshot();
        for (int i = 0; i < loaded_rows; i++) {
            double score = score_iforest_ts(&forest, records[i].features);
            int is_anomaly = (score >= thresh) ? 1 : 0;
            if (i >= train_rows) {
                int actual = records[i].label;
                if (is_anomaly && actual) tp++; else if (is_anomaly && !actual) fp++;
                else if (!is_anomaly && !actual) tn++; else if (!is_anomaly && actual) fn++;
            }
        }
        HardwareStats end_stats = get_hardware_snapshot();
        evaluate_and_log(results_csv, "Isolation Forest", thresh, tp, fp, tn, fn, test_rows, start_stats, end_stats);
    }

    fit_multi_algos_ts(records, train_rows);

    const char* algos[5] = {"Euclidean", "Manhattan", "Mahalanobis", "EpsDensity", "Hypercube"};
    
    for (int a = 0; a < 5; a++) {
        double t_start = 1.0, t_end = 5.0, t_step = 1.0;
        if (a == 1) { t_start = 2.0; t_end = 10.0; t_step = 2.0; }
        else if (a == 3) { t_start = 0.1; t_end = 0.5; t_step = 0.1; }
        else if (a == 4) { t_start = 0.5; t_end = 2.5; t_step = 0.5; }

        for (double thresh = t_start; thresh <= t_end; thresh += t_step) {
            int tp = 0, fp = 0, tn = 0, fn = 0;
            HardwareStats start_stats = get_hardware_snapshot();
            for (int i = 0; i < loaded_rows; i++) {
                double score = 0;
                if (a == 0) score = score_centroid_euclidean_ts(&records[i]);
                else if (a == 1) score = score_centroid_manhattan_ts(&records[i]);
                else if (a == 2) score = score_mahalanobis_ts(&records[i]);
                else if (a == 3) score = score_epsilon_density_ts(records, i, loaded_rows, 1.5);
                else if (a == 4) score = score_hypercube_ts(&records[i]);
                
                int is_anomaly = (score >= thresh) ? 1 : 0;
                if (i >= train_rows) {
                    int actual = records[i].label;
                    if (is_anomaly && actual) tp++; else if (is_anomaly && !actual) fp++;
                    else if (!is_anomaly && !actual) tn++; else if (!is_anomaly && actual) fn++;
                }
            }
            HardwareStats end_stats = get_hardware_snapshot();
            evaluate_and_log(results_csv, algos[a], thresh, tp, fp, tn, fn, test_rows, start_stats, end_stats);
        }
    }

    free(records);
    fclose(results_csv);
    printf("--- Time Series Sweep Complete ---\n");
    return 0;
}