// File: holt_winters_ts.c
// Implements anomaly detection edge algorithms.

#include <math.h>
#include <stdlib.h>
#include "config_ts.h"

typedef struct {
    double level;
    double trend;
    double seasonal[HW_SEASON_LENGTH];
    int season_idx;
    
    double residual_mean;
    double residual_variance;
    int n_updates;
} HoltWintersTS;

void init_hw_ts(HoltWintersTS* hw, double initial_val) {
    hw->level = initial_val;
    hw->trend = 0.0;
    for(int i = 0; i < HW_SEASON_LENGTH; i++) {
        hw->seasonal[i] = 1.0; 
    }
    hw->season_idx = 0;
    hw->residual_mean = 0.0;
    hw->residual_variance = 0.0;
    hw->n_updates = 0;
}

int evaluate_hw_ts_anomaly(HoltWintersTS* hw, double current_val, double threshold) {
    double current_season = hw->seasonal[hw->season_idx];
    double forecast = (hw->level + hw->trend) * current_season;
    
    double residual = fabs(current_val - forecast);
    
    hw->n_updates++;
    double delta = residual - hw->residual_mean;
    hw->residual_mean += delta / hw->n_updates;
    double delta2 = residual - hw->residual_mean;
    hw->residual_variance += delta * delta2;
    
    double std_dev = sqrt(hw->residual_variance / hw->n_updates);
    
    double prev_level = hw->level;
    hw->level = HW_ALPHA * (current_val / current_season) + (1 - HW_ALPHA) * (prev_level + hw->trend);
    hw->trend = HW_BETA * (hw->level - prev_level) + (1 - HW_BETA) * hw->trend;
    hw->seasonal[hw->season_idx] = HW_GAMMA * (current_val / hw->level) + (1 - HW_GAMMA) * current_season;

    hw->season_idx = (hw->season_idx + 1) % HW_SEASON_LENGTH;

    if (hw->n_updates > HW_SEASON_LENGTH && std_dev > 0) {
        double z_score = residual / std_dev;
        if (z_score > threshold) return 1;
    }
    
    return 0;
}