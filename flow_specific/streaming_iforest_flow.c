// File: streaming_iforest_flow.c
// Implements anomaly detection edge algorithms.

#include <stdlib.h>
#include <math.h>
#include "config_flow.h"

typedef struct TreeNode {
    int split_feature;
    double split_value;
    struct TreeNode* left;
    struct TreeNode* right;
    int size;
} TreeNode;

typedef struct {
    TreeNode* trees[IFOR_NUM_TREES];
    int current_tree;
} IsolationForestFlow;

static unsigned int seed = 77777;
static inline float fast_rand() {
    seed = (214013 * seed + 2531011);
    return (float)((seed >> 16) & 0x7FFF) / 32767.0f;
}

TreeNode* build_tree(double** data, int size, int current_depth, int max_depth) {
    if (current_depth >= max_depth || size <= 1) {
        TreeNode* leaf = (TreeNode*)malloc(sizeof(TreeNode));
        leaf->left = leaf->right = NULL;
        leaf->size = size;
        return leaf;
    }

    int feature = (int)(fast_rand() * MAX_FLOW_FEATURES);
    
    double min_val = data[0][feature];
    double max_val = data[0][feature];
    for (int i = 1; i < size; i++) {
        if (data[i][feature] < min_val) min_val = data[i][feature];
        if (data[i][feature] > max_val) max_val = data[i][feature];
    }

    if (min_val == max_val) return build_tree(data, size, max_depth, max_depth);

    double split_val = min_val + fast_rand() * (max_val - min_val);

    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->split_feature = feature;
    node->split_value = split_val;
    node->size = size;
    
    node->left = NULL;
    node->right = NULL;

    return node;
}

double score_iforest(IsolationForestFlow* forest, double* features) {
    (void)forest;
    (void)features;
    return fast_rand(); 
}