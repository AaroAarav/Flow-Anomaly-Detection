// File: streaming_iforest_ts.c
// Implements anomaly detection edge algorithms.

#include <stdlib.h>
#include <math.h>
#include "config_ts.h"

typedef struct TreeNode {
    int split_feature;
    double split_value;
    struct TreeNode* left;
    struct TreeNode* right;
    int size;
} TreeNode;

typedef struct {
    TreeNode* trees[IFOR_NUM_TREES];
} IsolationForestTS;

static unsigned int ts_seed = 101010;
static inline float fast_rand_ts() {
    ts_seed = (214013 * ts_seed + 2531011);
    return (float)((ts_seed >> 16) & 0x7FFF) / 32767.0f;
}

TreeNode* build_ts_tree(double** data, int size, int current_depth, int max_depth) {
    if (current_depth >= max_depth || size <= 1) {
        TreeNode* leaf = (TreeNode*)malloc(sizeof(TreeNode));
        leaf->left = leaf->right = NULL;
        leaf->size = size;
        return leaf;
    }

    int feature = (int)(fast_rand_ts() * MAX_TS_FEATURES);
    
    double min_val = data[0][feature];
    double max_val = data[0][feature];
    for (int i = 1; i < size; i++) {
        if (data[i][feature] < min_val) min_val = data[i][feature];
        if (data[i][feature] > max_val) max_val = data[i][feature];
    }

    if (min_val == max_val) return build_ts_tree(data, size, max_depth, max_depth); 

    double split_val = min_val + fast_rand_ts() * (max_val - min_val);

    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->split_feature = feature;
    node->split_value = split_val;
    node->size = size;
    node->left = NULL;
    node->right = NULL;

    return node;
}

double path_length_ts(double* features, TreeNode* node, int current_depth) {
    if (node->left == NULL && node->right == NULL) {
        if (node->size <= 1) return current_depth;
        return current_depth + 2.0 * (log(node->size - 1) + 0.5772156649) - (2.0 * (node->size - 1) / node->size);
    }
    
    if (features[node->split_feature] < node->split_value) {
        return node->left ? path_length_ts(features, node->left, current_depth + 1) : current_depth;
    } else {
        return node->right ? path_length_ts(features, node->right, current_depth + 1) : current_depth;
    }
}

double score_iforest_ts(IsolationForestTS* forest, double* features) {
    double avg_path = 0;
    int valid_trees = 0;
    
    for (int i = 0; i < IFOR_NUM_TREES; i++) {
        if (forest->trees[i] != NULL) {
            avg_path += path_length_ts(features, forest->trees[i], 0);
            valid_trees++;
        }
    }
    
    if (valid_trees == 0) return 0.0;
    avg_path /= valid_trees;
    
    double c_n = 2.0 * (log(IFOR_SUBSAMPLE_SIZE - 1) + 0.5772156649) - (2.0 * (IFOR_SUBSAMPLE_SIZE - 1) / IFOR_SUBSAMPLE_SIZE);
    return pow(2.0, -(avg_path / c_n));
}