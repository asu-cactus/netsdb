//
// Created by venkateshgunda on 4/13/22.
//

#ifndef NETSDB_LOGISTIC_REGRESSION_H
#define NETSDB_LOGISTIC_REGRESSION_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include <eigen3/Eigen/Dense>

using namespace pdb;

double sigmoid(double x);
double outLabel(double x);
void randomGen(Handle<Vector<double>> vec);

class Logistic_Regression : public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
    private:
        int numFeatures;
        int numLabels;
        int numNeurons;
        uint32_t sizeBatch;
        Handle<Vector<double>> weights = nullptr; // (numFeatures * numNeurons, 1)
        Handle<Vector<double>> biases = nullptr; // (numNeurons, 1)
        Handle<Vector<double>> output = nullptr; // (sizeBatch * numNeurons, 1)

    public:
        ENABLE_DEEP_COPY

        Logistic_Regression() {}

        Logistic_Regression(int numFeatures, int numLabels, int numNeurons, uint32_t sizeBatch) {
            this->numFeatures = numFeatures;
            this->numLabels = numLabels;
            this->numNeurons = numNeurons;
            this->sizeBatch = sizeBatch;
            this->weights = makeObject<Vector<double>>(numFeatures*numNeurons, numFeatures*numNeurons);
            randomGen(weights);
            this->biases = makeObject<Vector<double>>(numNeurons, numNeurons);
            randomGen(biases);
            this->output = makeObject<Vector<double>>(sizeBatch*numNeurons,sizeBatch*numNeurons);
            randomGen(output);
        }

        // Implement the Virtual Function of SelectionComp
        Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
            return makeLambda(checkMe, [](Handle<FFMatrixBlock> &checkMe) {return true; });
        }

        // Implement the Virtual Function of SelectionComp
        Lambda<Handle<FFMatrixBlock>> getProjection(Handle<FFMatrixBlock> in) override {
            return makeLambda(in, [this](Handle<FFMatrixBlock> &in) {
                // load the metadata
                uint32_t inNumRow = in->getRowNums();
                uint32_t inNumCol = in->getColNums();
                uint32_t inBlockRowIndex = in->getBlockRowIndex();
                uint32_t inBlockColIndex = in->getBlockColIndex();

                if(sizeBatch!=inNumRow) {
                    std::cout << "Batch Size doesn't match: " << sizeBatch << ", " << inNumRow << std::endl;
                    exit(1);
                }

                double *inData = in->getValue().rawData->c_ptr();
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> x(inData, numFeatures, sizeBatch);
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> w(weights->c_ptr(), numNeurons, numFeatures);
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> b(biases->c_ptr(), numNeurons, 1);
                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> y(output->c_ptr(), numNeurons, sizeBatch);

                // Logistic Regression Function: f(x) = sigmoid(w*x + b)
                y = w * x;
                y = y.colwise() + b.col(0);
                y = y.unaryExpr(&sigmoid);
                y = y.unaryExpr(&outLabel);

                pdb::Handle<FFMatrixBlock> resultFFMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, numLabels, sizeBatch, false);

                double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();

                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> resultMatrix(outData, numLabels, sizeBatch);

                resultMatrix = y;

                return resultFFMatrixBlock;
            });
        }
};

#endif //NETSDB_LOGISTIC_REGRESSION_H
