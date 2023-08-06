#ifndef SEMANTIC_CLASSIFIER_CC
#define SEMANTIC_CLASSIFIER_CC

// This program is generated by machine at a random time

#include "SemanticClassifier.h"
#include "GetVTable.h"

GET_V_TABLE(SemanticClassifier)

float sigmoid(float x) { return 1 / (1 + exp(-x)); }

float relu(float x) { return std::max(0.0f, x); }

float outLabel(float x) {
    float threshold = 0.5;
    if (x > threshold) {
        return 1;
    } else {
        return 0;
    }
}

#endif
