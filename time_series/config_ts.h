#ifndef CONFIG_TS_H
#define CONFIG_TS_H

#define MAX_ROWS 3000 
#define MAX_TS_FEATURES 4

// HYPERPARAMETER DOCUMENTATION
// ==========================================
// HYPERPARAMETER:
// IFOR_NUM_TREES controls the number of trees in the Streaming Isolation Forest.
// Time-series data often has high variance, requiring more trees to isolate temporal anomalies.
// Higher values improve anomaly detection accuracy but increase CPU usage and memory consumption.
// Lightweight recommendation: 25
// High-accuracy recommendation: 100
#define IFOR_NUM_TREES 100

// HYPERPARAMETER:
// IFOR_SUBSAMPLE_SIZE dictates the mini-batch size for streaming tree building.
// Higher values yield better bounds but increase the memory footprint of the active window.
// Lightweight recommendation: 128
// High-accuracy recommendation: 512
#define IFOR_SUBSAMPLE_SIZE 512

// HYPERPARAMETER:
// LOF_K_NEIGHBORS controls the local neighborhood size for density estimation in LOF.
// Impacts distance matrix computation time exponentially.
// Lightweight recommendation: 10 (Euclidean distance is more expensive)
// High-accuracy recommendation: 35
#define LOF_K_NEIGHBORS 35
#define ABOD_K_NEIGHBORS 15

// HYPERPARAMETER:
// LOF_CONTAMINATION defines the anomaly score cutoff threshold for LOF.
// Higher values mean fewer alerts (stricter anomaly definition).
// Lightweight recommendation: 1.5
// High-accuracy recommendation: 2.5
#define LOF_CONTAMINATION 1.5

// HYPERPARAMETER:
// HW_SEASON_LENGTH defines the periodicity for Holt-Winters multiplicative seasonality.
// Affects the ring buffer size. Crucial for detecting off-hours anomalies.
// Lightweight recommendation: 10 (e.g., 10 connection rolling window)
// High-accuracy recommendation: 60
#define HW_SEASON_LENGTH 60

// HYPERPARAMETER:
// HW_ALPHA controls the smoothing factor for the level (trend base).
// Higher values give more weight to recent observations, prioritizing short-term memory.
// Lightweight recommendation: 0.3
// High-accuracy recommendation: 0.5
#define HW_ALPHA 0.5

// HYPERPARAMETER:
// HW_BETA controls the trend smoothing factor (additive trend).
// Lightweight recommendation: 0.1
// High-accuracy recommendation: 0.2
#define HW_BETA 0.2

// HYPERPARAMETER:
// HW_GAMMA controls the seasonal smoothing factor (multiplicative).
// Lightweight recommendation: 0.2
// High-accuracy recommendation: 0.4
#define HW_GAMMA 0.4

// HYPERPARAMETER:
// HW_RESIDUAL_THRESHOLD controls the allowed deviation before flagging an anomaly.
// Lightweight recommendation: 3.0 (standard deviations)
// High-accuracy recommendation: 2.0
#define HW_RESIDUAL_THRESHOLD 2.0

typedef struct {
    long cpu_time_us;
    long ram_kb;
    long disk_io_bytes;
    double timestamp_ms;
} HardwareStats;

#endif
