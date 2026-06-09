// File: lof_ts.c
// Implements anomaly detection edge algorithms.

#include <math.h>
#include <stdlib.h>
#include "config_ts.h"

static inline double euclidean_distance(double* a, double* b) {
    double dist = 0.0;
    for (int i = 0; i < MAX_TS_FEATURES; i++) {
        double diff = a[i] - b[i];
        dist += (diff * diff);
    }
    return sqrt(dist);
}

double compute_lrd_ts(double distance_matrix[MAX_ROWS][MAX_ROWS], int target_idx, int num_rows) {
    double reach_dist_sum = 0.0;
    int valid_neighbors = 0;

    for (int i = 0; i < num_rows; i++) {
        if (i != target_idx) {
            reach_dist_sum += distance_matrix[target_idx][i];
            valid_neighbors++;
            if (valid_neighbors >= LOF_K_NEIGHBORS) break;
        }
    }
    
    if (reach_dist_sum == 0.0 || valid_neighbors == 0) return 0.0;
    return (double)valid_neighbors / reach_dist_sum;
}

double compute_lof_score_ts(double* lrd_array, int target_idx, int num_rows) {
    double lrd_ratio_sum = 0.0;
    int valid_neighbors = 0;

    for (int i = 0; i < num_rows; i++) {
        if (i != target_idx && lrd_array[i] > 0) {
            lrd_ratio_sum += lrd_array[i] / lrd_array[target_idx];
            valid_neighbors++;
            if (valid_neighbors >= LOF_K_NEIGHBORS) break;
        }
    }
    
    if (valid_neighbors == 0) return 1.0;
    return lrd_ratio_sum / (double)valid_neighbors;
}