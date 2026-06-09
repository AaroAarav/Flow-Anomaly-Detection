#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config_flow.h"

// --- Data Structures & External Parsers ---
typedef struct { double features[MAX_FLOW_FEATURES]; int label; } FlowRecord;
extern int parse_flow_csv(const char* filepath, FlowRecord* records);

// --- Hardware Metrics Hook ---
extern HardwareStats get_hardware_snapshot();

// --- External Algorithm Structs & Signatures ---
typedef struct { 
    double level; double trend; double seasonal[HW_SEASON_LENGTH]; int season_idx; 
} HoltWintersState;
extern void init_hw(HoltWintersState* hw, double initial_val);
extern int evaluate_hw_flow_anomaly(HoltWintersState* hw, double current_val, double threshold);

typedef struct { void* trees[IFOR_NUM_TREES]; int current_tree; } IsolationForestFlow;
extern double score_iforest(IsolationForestFlow* forest, double* features);

extern void fit_multi_algos_flow(FlowRecord* records, int train_rows);
extern double score_centroid_euclidean_flow(FlowRecord* record);
extern double score_centroid_manhattan_flow(FlowRecord* record);
extern double score_mahalanobis_flow(FlowRecord* record);
extern double score_epsilon_density_flow(FlowRecord* records, int target_idx, int num_rows, double epsilon);
extern double score_hypercube_flow(FlowRecord* record);

// --- Metrics Helper ---
void evaluate_and_log(FILE* csv, const char* algo, double thresh, int tp, int fp, int tn, int fn, int total_test_rows, HardwareStats start, HardwareStats end) {
    double accuracy = (total_test_rows > 0) ? (double)(tp + tn) / total_test_rows : 0.0;
    double precision = (tp + fp > 0) ? (double)tp / (tp + fp) : 0.0;
    double recall = (tp + fn > 0) ? (double)tp / (tp + fn) : 0.0;
    double f1 = (precision + recall > 0) ? 2.0 * (precision * recall) / (precision + recall) : 0.0;
    
    // Calculate Hardware Deltas
    double latency = end.timestamp_ms - start.timestamp_ms;
    long cpu_usage = end.cpu_time_us - start.cpu_time_us;
    long disk_io = end.disk_io_bytes - start.disk_io_bytes;
    long peak_ram = end.ram_kb; // RAM is tracked as peak

    fprintf(csv, "%s,%.2f,%d,%d,%d,%d,%.4f,%.4f,%.4f,%.4f,%.2f,%ld,%ld,%ld\n", 
            algo, thresh, tp, fp, tn, fn, accuracy, precision, recall, f1, latency, cpu_usage, peak_ram, disk_io);
            
    printf("[%s] Thresh %8.2f -> F1: %.4f | Latency: %6.2fms | CPU: %8ldus | RAM: %ldKB\n", 
            algo, thresh, f1, latency, cpu_usage, peak_ram);
}

int main(int argc, char *argv[]) {
    char* dataset_path = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1) { 
        if (opt == 'd') dataset_path = optarg; 
    }
    
    if (!dataset_path) {
        fprintf(stderr, "[ERROR] Usage: %s -d <dataset.csv>\n", argv[0]);
        return -1;
    }

    FlowRecord* records = (FlowRecord*)malloc(sizeof(FlowRecord) * MAX_ROWS);
    int loaded_rows = parse_flow_csv(dataset_path, records);
    
    // 80/20 Train/Test Split
    int train_rows = (int)(loaded_rows * 0.8);
    int test_rows = loaded_rows - train_rows;
    printf("[INFO] Train/Test Split: 80/20 (%d Training Burn-in, %d Testing)\n", train_rows, test_rows);
    
    FILE* results_csv = fopen("../results/flow_param_sweep.csv", "w");
    if(!results_csv) {
        perror("[ERROR] Could not create results CSV");
        free(records);
        return -1;
    }
    
    // NEW CSV HEADERS
    fprintf(results_csv, "Algorithm,Threshold,TP,FP,TN,FN,Accuracy,Precision,Recall,F1_Score,Latency_ms,CPU_us,Peak_RAM_KB,Disk_IO_Bytes\n");
    printf("\n--- Starting Flow-Specific Hyperparameter & Hardware Sweep ---\n");

    // ==========================================
    // 1. HOLT-WINTERS SWEEP 
    // Tracking SYN Flags (features[2]) to detect scans/floods
    // ==========================================
    for (double thresh = 10.0; thresh <= 500.0; thresh += 50.0) {
        HoltWintersState hw;
        // Updated to features[2] for SYN Flag Count
        if (loaded_rows > 0) init_hw(&hw, records[0].features[2]); 
        int tp = 0, fp = 0, tn = 0, fn = 0;

        HardwareStats start_stats = get_hardware_snapshot();
        for (int i = 0; i < loaded_rows; i++) {
            // Updated to features[2] to actively evaluate SYN Flags
            int is_anomaly = evaluate_hw_flow_anomaly(&hw, records[i].features[2], thresh);
            
            if (i >= train_rows) {
                int actual = records[i].label;
                if (is_anomaly && actual) tp++; 
                else if (is_anomaly && !actual) fp++;
                else if (!is_anomaly && !actual) tn++; 
                else if (!is_anomaly && actual) fn++;
            }
        }
        HardwareStats end_stats = get_hardware_snapshot();
        evaluate_and_log(results_csv, "Holt-Winters", thresh, tp, fp, tn, fn, test_rows, start_stats, end_stats);
    }

    // ==========================================
    // 2. ISOLATION FOREST SWEEP
    // ==========================================
    IsolationForestFlow forest;
    memset(&forest, 0, sizeof(IsolationForestFlow)); 
    
    for (double thresh = 0.45; thresh <= 0.70; thresh += 0.05) {
        int tp = 0, fp = 0, tn = 0, fn = 0;
        
        HardwareStats start_stats = get_hardware_snapshot();
        for (int i = 0; i < loaded_rows; i++) {
            double score = score_iforest(&forest, records[i].features);
            int is_anomaly = (score >= thresh) ? 1 : 0;
            
            if (i >= train_rows) {
                int actual = records[i].label;
                if (is_anomaly && actual) tp++; 
                else if (is_anomaly && !actual) fp++;
                else if (!is_anomaly && !actual) tn++; 
                else if (!is_anomaly && actual) fn++;
            }
        }
        HardwareStats end_stats = get_hardware_snapshot();
        evaluate_and_log(results_csv, "Isolation Forest", thresh, tp, fp, tn, fn, test_rows, start_stats, end_stats);
    }

    fit_multi_algos_flow(records, train_rows);

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
                if (a == 0) score = score_centroid_euclidean_flow(&records[i]);
                else if (a == 1) score = score_centroid_manhattan_flow(&records[i]);
                else if (a == 2) score = score_mahalanobis_flow(&records[i]);
                else if (a == 3) score = score_epsilon_density_flow(records, i, loaded_rows, 1.5);
                else if (a == 4) score = score_hypercube_flow(&records[i]);
                
                int is_anomaly = (score >= thresh) ? 1 : 0;
                
                if (i >= train_rows) {
                    int actual = records[i].label;
                    if (is_anomaly && actual) tp++; 
                    else if (is_anomaly && !actual) fp++;
                    else if (!is_anomaly && !actual) tn++; 
                    else if (!is_anomaly && actual) fn++;
                }
            }
            HardwareStats end_stats = get_hardware_snapshot();
            evaluate_and_log(results_csv, algos[a], thresh, tp, fp, tn, fn, test_rows, start_stats, end_stats);
        }
    }

    // Cleanup
    free(records);
    fclose(results_csv);
    printf("--- Flow Sweep Complete ---\n");
    printf("[SUCCESS] Results saved to ../results/flow_param_sweep.csv\n");
    return 0;
}