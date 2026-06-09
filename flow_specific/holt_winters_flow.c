#include "config_flow.h"
#include <math.h>

typedef struct {
    double level;
    double trend;
    double seasonal[HW_SEASON_LENGTH];
    int season_idx;
} HoltWintersState;

void init_hw(HoltWintersState* hw, double initial_val) {
    hw->level = initial_val;
    hw->trend = 0.0;
    for(int i=0; i < HW_SEASON_LENGTH; i++) {
        hw->seasonal[i] = 1.0; // Multiplicative initialization
    }
    hw->season_idx = 0;
}

// Online update step for streaming pipeline
double update_hw_score(HoltWintersState* hw, double current_flow_bytes_sec) {
    double prev_level = hw->level;
    double prev_trend = hw->trend;
    double current_season = hw->seasonal[hw->season_idx];

    // Multiplicative seasonality update
    hw->level = HW_ALPHA * (current_flow_bytes_sec / current_season) + (1 - HW_ALPHA) * (prev_level + prev_trend);
    hw->trend = HW_BETA * (hw->level - prev_level) + (1 - HW_BETA) * prev_trend;
    hw->seasonal[hw->season_idx] = HW_GAMMA * (current_flow_bytes_sec / hw->level) + (1 - HW_GAMMA) * current_season;

    // Advance season
    hw->season_idx = (hw->season_idx + 1) % HW_SEASON_LENGTH;

    // Forecast next value
    double forecast = (hw->level + hw->trend) * hw->seasonal[hw->season_idx];
    
    // Residual error represents the anomaly score
    double residual = fabs(current_flow_bytes_sec - forecast);
    return residual;
}
// Adaptive anomaly scoring based on dynamic residual thresholds
int evaluate_hw_flow_anomaly(HoltWintersState* hw, double current_val, double threshold) {
    double current_season = hw->seasonal[hw->season_idx];
    double forecast = (hw->level + hw->trend) * current_season;
    
    double residual = fabs(current_val - forecast);
    
    // Online state update
    double prev_level = hw->level;
    hw->level = HW_ALPHA * (current_val / current_season) + (1 - HW_ALPHA) * (prev_level + hw->trend);
    hw->trend = HW_BETA * (hw->level - prev_level) + (1 - HW_BETA) * hw->trend;
    hw->seasonal[hw->season_idx] = HW_GAMMA * (current_val / hw->level) + (1 - HW_GAMMA) * current_season;

    hw->season_idx = (hw->season_idx + 1) % HW_SEASON_LENGTH;

    // Check if the residual error exceeds the sweeping threshold
    if (residual > threshold) return 1; 
    
    return 0; // Normal
}