// File: abod_ts.c
// Implements anomaly detection edge algorithms.

#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "config_ts.h"

typedef struct {
    double features[MAX_TS_FEATURES];
    int label;
} TSRecord;

double compute_abod_score_ts(TSRecord* records, int target_idx, int num_rows) {
    int neighbors[ABOD_K_NEIGHBORS];
    int n_count = 0;

    double dists[MAX_ROWS];
    for (int i = 0; i < num_rows; i++) {
        if (i == target_idx) {
            dists[i] = DBL_MAX;
        } else {
            double dist_sq = 0.0;
            for (int f = 0; f < MAX_TS_FEATURES; f++) {
                double diff = records[target_idx].features[f] - records[i].features[f];
                dist_sq += diff * diff;
            }
            dists[i] = dist_sq;
        }
    } 
    
    for (int k = 0; k < ABOD_K_NEIGHBORS && k < num_rows - 1; k++) {
        double min_d = DBL_MAX;
        int min_idx = -1;
        for (int i = 0; i < num_rows; i++) {
            if (dists[i] < min_d) {
                min_d = dists[i];
                min_idx = i;
            }
        }
        if (min_idx != -1) {
            neighbors[n_count++] = min_idx;
            dists[min_idx] = DBL_MAX; 
        }
    }

    if (n_count < 2) return 0.0;

    double sum_angle = 0.0;
    double sum_angle_sq = 0.0;
    int pairs = 0;

    for (int i = 0; i < n_count; i++) {
        for (int j = i + 1; j < n_count; j++) {
            int n1 = neighbors[i];
            int n2 = neighbors[j];
            
            double dot = 0.0;
            double normAB = 0.0;
            double normAC = 0.0;
            for (int f = 0; f < MAX_TS_FEATURES; f++) {
                double ab = records[n1].features[f] - records[target_idx].features[f];
                double ac = records[n2].features[f] - records[target_idx].features[f];
                dot += ab * ac;
                normAB += ab * ab;
                normAC += ac * ac;
            }
            
            if (normAB == 0 || normAC == 0) continue;
            
            double denominator = normAB * normAC;
            double angle = dot / denominator;
            sum_angle += angle;
            sum_angle_sq += angle * angle;
            pairs++;
        }
    }

    if (pairs == 0) return 0.0;
    double mean = sum_angle / pairs;
    double variance = (sum_angle_sq / pairs) - (mean * mean);
    
    return 1.0 / (variance + 1e-9);
}
