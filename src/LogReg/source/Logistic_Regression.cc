#ifndef NETSDB_LOGISTIC_REGRESSION_CC
#define NETSDB_LOGISTIC_REGRESSION_CC

#include "Logistic_Regression.h"
#include "GetVTable.h"
#include <algorithm>
#include <random>

GET_V_TABLE(Logistic_Regression)

double sigmoid(double x) {return 1/(1+exp(-x));}

double outLabel(double x) {
    double threshold = 0.5;
    if(x>threshold) {
        return 1;
    }
    else {
        return 0;
    }
}

void randomGen(Handle<Vector<double>> vec) {
    std::random_device rd;

    std::mt19937 e2(rd());

    std::uniform_real_distribution<> distp(0.0001, 0.5);
    std::uniform_real_distribution<> distn(-0.5, -0.0001);

    auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());

    for(int i=0;i<vec->size();i++) {
        (*vec)[i] = (bool)gen() ? distn(e2) : distp(e2);
    }
}

#endif