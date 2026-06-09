#ifndef CONFIG_FLOW_H
#define CONFIG_FLOW_H

#define MAX_ROWS 3000 
#define MAX_FLOW_FEATURES 4

// HYPERPARAMETER DOCUMENTATION
// ==========================================
// HYPERPARAMETER:
// IFOR_NUM_TREES controls the number of trees in the Streaming Isolation Forest.
// Flow data requires fewer trees than raw time-series due to pre-aggregated features.
// Higher values improve accuracy but linearly increase CPU and memory usage.
// Lightweight recommendation: 15
// High-accuracy recommendation: 50
#define IFOR_NUM_TREES 50

// HYPERPARAMETER:
// IFOR_SUBSAMPLE_SIZE dictates the mini-batch size for streaming tree building.
// Higher values yield better bounds but increase the memory footprint of the active window.
// Lightweight recommendation: 64
// High-accuracy recommendation: 256
#define IFOR_SUBSAMPLE_SIZE 256

// HYPERPARAMETER:
// LOF_K_NEIGHBORS controls the local neighborhood size for density estimation in LOF.
// Impacts distance matrix computation time exponentially.
// Lightweight recommendation: 5 (Fast Manhattan distance calculations)
// High-accuracy recommendation: 20
#define LOF_K_NEIGHBORS 20
#define ABOD_K_NEIGHBORS 15

// HYPERPARAMETER:
// LOF_CONTAMINATION defines the threshold for flagging an anomaly.
// Lightweight recommendation: 1.5
// High-accuracy recommendation: 2.0 (stricter)
#define LOF_CONTAMINATION 2.0

// HYPERPARAMETER:
// HW_SEASON_LENGTH defines the periodicity for Holt-Winters applied to Flow Bytes/s.
// Lightweight recommendation: 5 (micro-burst detection)
// High-accuracy recommendation: 20
#define HW_SEASON_LENGTH 20

// HYPERPARAMETER:
// HW_ALPHA controls the smoothing factor for the level (trend base).
// Lightweight recommendation: 0.4
// High-accuracy recommendation: 0.2
#define HW_ALPHA 0.2

// HYPERPARAMETER:
// HW_BETA controls the trend smoothing factor.
#define HW_BETA 0.1

// HYPERPARAMETER:
// HW_GAMMA controls the seasonal smoothing factor.
#define HW_GAMMA 0.1

typedef struct {
    long cpu_time_us;
    long ram_kb;
    long disk_io_bytes;
    double timestamp_ms;
} HardwareStats;

#endif
