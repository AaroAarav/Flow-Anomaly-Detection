#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include "config_flow.h"

typedef struct {
    double features[MAX_FLOW_FEATURES];
    int label;
} FlowRecord;

static double flow_centroid[MAX_FLOW_FEATURES] = {0};
static double flow_min_bound[MAX_FLOW_FEATURES];
static double flow_max_bound[MAX_FLOW_FEATURES];
static double flow_inv_cov[MAX_FLOW_FEATURES][MAX_FLOW_FEATURES] = {0};

static int invert_4x4(double m[4][4], double invOut[4][4]) {
    double inv[16], det;
    double m_1d[16];
    for(int i=0; i<4; i++) for(int j=0; j<4; j++) m_1d[i*4+j] = m[i][j];

    inv[0] = m_1d[5]  * m_1d[10] * m_1d[15] - m_1d[5]  * m_1d[11] * m_1d[14] - m_1d[9]  * m_1d[6]  * m_1d[15] + m_1d[9]  * m_1d[7]  * m_1d[14] +m_1d[13] * m_1d[6]  * m_1d[11] - m_1d[13] * m_1d[7]  * m_1d[10];
    inv[4] = -m_1d[4]  * m_1d[10] * m_1d[15] + m_1d[4]  * m_1d[11] * m_1d[14] + m_1d[8]  * m_1d[6]  * m_1d[15] - m_1d[8]  * m_1d[7]  * m_1d[14] - m_1d[12] * m_1d[6]  * m_1d[11] + m_1d[12] * m_1d[7]  * m_1d[10];
    inv[8] = m_1d[4]  * m_1d[9] * m_1d[15] - m_1d[4]  * m_1d[11] * m_1d[13] - m_1d[8]  * m_1d[5] * m_1d[15] + m_1d[8]  * m_1d[7] * m_1d[13] + m_1d[12] * m_1d[5] * m_1d[11] - m_1d[12] * m_1d[7] * m_1d[9];
    inv[12] = -m_1d[4]  * m_1d[9] * m_1d[14] + m_1d[4]  * m_1d[10] * m_1d[13] +m_1d[8]  * m_1d[5] * m_1d[14] - m_1d[8]  * m_1d[6] * m_1d[13] - m_1d[12] * m_1d[5] * m_1d[10] + m_1d[12] * m_1d[6] * m_1d[9];
    inv[1] = -m_1d[1]  * m_1d[10] * m_1d[15] + m_1d[1]  * m_1d[11] * m_1d[14] + m_1d[9]  * m_1d[2] * m_1d[15] - m_1d[9]  * m_1d[3] * m_1d[14] - m_1d[13] * m_1d[2] * m_1d[11] + m_1d[13] * m_1d[3] * m_1d[10];
    inv[5] = m_1d[0]  * m_1d[10] * m_1d[15] - m_1d[0]  * m_1d[11] * m_1d[14] - m_1d[8]  * m_1d[2] * m_1d[15] + m_1d[8]  * m_1d[3] * m_1d[14] + m_1d[12] * m_1d[2] * m_1d[11] - m_1d[12] * m_1d[3] * m_1d[10];
    inv[9] = -m_1d[0]  * m_1d[9] * m_1d[15] + m_1d[0]  * m_1d[11] * m_1d[13] + m_1d[8]  * m_1d[1] * m_1d[15] - m_1d[8]  * m_1d[3] * m_1d[13] - m_1d[12] * m_1d[1] * m_1d[11] + m_1d[12] * m_1d[3] * m_1d[9];
    inv[13] = m_1d[0]  * m_1d[9] * m_1d[14] - m_1d[0]  * m_1d[10] * m_1d[13] - m_1d[8]  * m_1d[1] * m_1d[14] + m_1d[8]  * m_1d[2] * m_1d[13] + m_1d[12] * m_1d[1] * m_1d[10] - m_1d[12] * m_1d[2] * m_1d[9];
    inv[2] = m_1d[1]  * m_1d[6] * m_1d[15] - m_1d[1]  * m_1d[7] * m_1d[14] - m_1d[5]  * m_1d[2] * m_1d[15] + m_1d[5]  * m_1d[3] * m_1d[14] + m_1d[13] * m_1d[2] * m_1d[7] - m_1d[13] * m_1d[3] * m_1d[6];
    inv[6] = -m_1d[0]  * m_1d[6] * m_1d[15] + m_1d[0]  * m_1d[7] * m_1d[14] + m_1d[4]  * m_1d[2] * m_1d[15] - m_1d[4]  * m_1d[3] * m_1d[14] - m_1d[12] * m_1d[2] * m_1d[7] + m_1d[12] * m_1d[3] * m_1d[6];
    inv[10] = m_1d[0]  * m_1d[5] * m_1d[15] - m_1d[0]  * m_1d[7] * m_1d[13] - m_1d[4]  * m_1d[1] * m_1d[15] + m_1d[4]  * m_1d[3] * m_1d[13] + m_1d[12] * m_1d[1] * m_1d[7] - m_1d[12] * m_1d[3] * m_1d[5];
    inv[14] = -m_1d[0]  * m_1d[5] * m_1d[14] + m_1d[0]  * m_1d[6] * m_1d[13] + m_1d[4]  * m_1d[1] * m_1d[14] - m_1d[4]  * m_1d[2] * m_1d[13] - m_1d[12] * m_1d[1] * m_1d[6] + m_1d[12] * m_1d[2] * m_1d[5];
    inv[3] = -m_1d[1] * m_1d[6] * m_1d[11] + m_1d[1] * m_1d[7] * m_1d[10] + m_1d[5] * m_1d[2] * m_1d[11] - m_1d[5] * m_1d[3] * m_1d[10] - m_1d[9] * m_1d[2] * m_1d[7] + m_1d[9] * m_1d[3] * m_1d[6];
    inv[7] = m_1d[0] * m_1d[6] * m_1d[11] - m_1d[0] * m_1d[7] * m_1d[10] - m_1d[4] * m_1d[2] * m_1d[11] + m_1d[4] * m_1d[3] * m_1d[10] + m_1d[8] * m_1d[2] * m_1d[7] - m_1d[8] * m_1d[3] * m_1d[6];
    inv[11] = -m_1d[0] * m_1d[5] * m_1d[11] + m_1d[0] * m_1d[7] * m_1d[9] + m_1d[4] * m_1d[1] * m_1d[11] - m_1d[4] * m_1d[3] * m_1d[9] - m_1d[8] * m_1d[1] * m_1d[7] + m_1d[8] * m_1d[3] * m_1d[5];
    inv[15] = m_1d[0] * m_1d[5] * m_1d[10] - m_1d[0] * m_1d[6] * m_1d[9] - m_1d[4] * m_1d[1] * m_1d[10] + m_1d[4] * m_1d[2] * m_1d[9] + m_1d[8] * m_1d[1] * m_1d[6] - m_1d[8] * m_1d[2] * m_1d[5];

    det = m_1d[0] * inv[0] + m_1d[1] * inv[4] + m_1d[2] * inv[8] + m_1d[3] * inv[12];
    if (det == 0) return 0;
    det = 1.0 / det;
    for (int i = 0; i < 16; i++) {
        invOut[i/4][i%4] = inv[i] * det;
    }
    return 1;
}

void fit_multi_algos_flow(FlowRecord* records, int train_rows) {
    if (train_rows <= 0) return;
    
    for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
        flow_centroid[f] = 0;
        flow_min_bound[f] = DBL_MAX;
        flow_max_bound[f] = -DBL_MAX;
    }

    for (int i = 0; i < train_rows; i++) {
        for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
            double v = records[i].features[f];
            flow_centroid[f] += v;
            if (v < flow_min_bound[f]) flow_min_bound[f] = v;
            if (v > flow_max_bound[f]) flow_max_bound[f] = v;
        }
    }
    for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
        flow_centroid[f] /= train_rows;
    }

    double cov[MAX_FLOW_FEATURES][MAX_FLOW_FEATURES] = {0};
    for (int i = 0; i < train_rows; i++) {
        for (int f1 = 0; f1 < MAX_FLOW_FEATURES; f1++) {
            for (int f2 = 0; f2 < MAX_FLOW_FEATURES; f2++) {
                cov[f1][f2] += (records[i].features[f1] - flow_centroid[f1]) * 
                               (records[i].features[f2] - flow_centroid[f2]);
            }
        }
    }
    for (int f1 = 0; f1 < MAX_FLOW_FEATURES; f1++) {
        for (int f2 = 0; f2 < MAX_FLOW_FEATURES; f2++) {
            cov[f1][f2] /= (train_rows - 1);
        }
    }
    
    if (MAX_FLOW_FEATURES == 4) {
        if (!invert_4x4(cov, flow_inv_cov)) {
            for(int i=0; i<4; i++) flow_inv_cov[i][i] = 1.0;
        }
    } else {
        for(int i=0; i<MAX_FLOW_FEATURES; i++) flow_inv_cov[i][i] = 1.0;
    }
}

double score_centroid_euclidean_flow(FlowRecord* record) {
    double dist = 0;
    for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
        double d = record->features[f] - flow_centroid[f];
        dist += d * d;
    }
    return sqrt(dist);
}

double score_centroid_manhattan_flow(FlowRecord* record) {
    double dist = 0;
    for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
        dist += fabs(record->features[f] - flow_centroid[f]);
    }
    return dist;
}

double score_mahalanobis_flow(FlowRecord* record) {
    double diff[MAX_FLOW_FEATURES];
    for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
        diff[f] = record->features[f] - flow_centroid[f];
    }
    
    double dist = 0;
    for (int i = 0; i < MAX_FLOW_FEATURES; i++) {
        double row_sum = 0;
        for (int j = 0; j < MAX_FLOW_FEATURES; j++) {
            row_sum += diff[j] * flow_inv_cov[j][i];
        }
        dist += row_sum * diff[i];
    }
    return sqrt(fabs(dist));
}

double score_epsilon_density_flow(FlowRecord* records, int target_idx, int num_rows, double epsilon) {
    int neighbors = 0;
    for (int i = 0; i < num_rows; i++) {
        if (i == target_idx) continue;
        double dist = 0;
        for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
            double d = records[target_idx].features[f] - records[i].features[f];
            dist += d * d;
        }
        if (sqrt(dist) <= epsilon) neighbors++;
    }
    if (neighbors == 0) return 1000.0;
    return 1.0 / neighbors;
}

double score_hypercube_flow(FlowRecord* record) {
    double dist_outside = 0;
    for (int f = 0; f < MAX_FLOW_FEATURES; f++) {
        if (record->features[f] < flow_min_bound[f]) {
            double d = flow_min_bound[f] - record->features[f];
            dist_outside += d;
        } else if (record->features[f] > flow_max_bound[f]) {
            double d = record->features[f] - flow_max_bound[f];
            dist_outside += d;
        }
    }
    return dist_outside;
}
