#ifndef FULLY_CONNECTED_NETWORK_H
#define FULLY_CONNECTED_NETWORK_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include <eigen3/Eigen/Dense>


using namespace pdb;

double sigmoid(double x);
double relu(double x);
double outLabel(double x);
void randomGen(Handle<Vector<double>> vec);

class FullyConnectedNetwork : public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  private:
    int numFeatures;
    int numLabels;
    int numNeurons;
    uint32_t sizeBatch;
    Handle<Vector<double>> weights1 = nullptr; //( numFeatures * numNeurons, 1);
    Handle<Vector<double>> bias1 = nullptr; //( numNeurons, 1);
    Handle<Vector<double>> output1 = nullptr; //( sizeBatch * numNeurons, 1);
    Handle<Vector<double>> weights2 = nullptr; //( numNeurons * numLabels, 1);
    Handle<Vector<double>> bias2 = nullptr; //( numLabels, 1);
    Handle<Vector<double>> output2 = nullptr; //( sizeBatch * numLabels, 1);

  public:
    ENABLE_DEEP_COPY

    FullyConnectedNetwork() {
    }

    FullyConnectedNetwork(int numFeatures, int numLabels, int numNeurons, uint32_t sizeBatch) {
        this->numFeatures = numFeatures;
	this->numLabels = numLabels;
	this->numNeurons = numNeurons;
	this->sizeBatch = sizeBatch;
	this->weights1 = makeObject<Vector<double>>(numFeatures*numNeurons, numFeatures*numNeurons);
        randomGen(weights1);
	this->bias1 = makeObject<Vector<double>>(numNeurons, numNeurons);
	randomGen(bias1);
	this->output1 = makeObject<Vector<double>>(sizeBatch * numNeurons, sizeBatch * numNeurons);
	randomGen(output1);
	this->weights2 = makeObject<Vector<double>>(numNeurons * numLabels, numNeurons * numLabels);
	randomGen(weights2);
	this->bias2 = makeObject<Vector<double>>(numLabels, numLabels);
	randomGen(bias2);
	this->output2 = makeObject<Vector<double>>(sizeBatch * numLabels, sizeBatch * numLabels);
	randomGen(output2);
    }

    Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
        return makeLambda(checkMe,
                          [](Handle<FFMatrixBlock> &checkMe) { return true; });
    }

    Lambda<Handle<FFMatrixBlock>>
    getProjection(Handle<FFMatrixBlock> in) override {
        return makeLambda(in, [this](Handle<FFMatrixBlock> &in) {
            // load the metadata
            uint32_t inNumRow = in->getRowNums();
            uint32_t inNumCol = in->getColNums();
            uint32_t inBlockRowIndex = in->getBlockRowIndex();
            uint32_t inBlockColIndex = in->getBlockColIndex();

            if (sizeBatch != inNumRow) { 
	        std::cout << "batch size doesn't match: " << sizeBatch << ", " << inNumRow << std::endl;
		exit(1);
	    }

            double *inData = in->getValue().rawData->c_ptr();
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                x0(inData, numFeatures, sizeBatch);
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                w0(weights1->c_ptr(), numNeurons, numFeatures);
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                b0(bias1->c_ptr(), numNeurons, 1);
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                y0(output1->c_ptr(), numNeurons, sizeBatch);

            y0 = w0 * x0;
            y0 = y0.colwise() + b0.col(0);
            y0 = y0.unaryExpr(&relu);

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                w1(weights2->c_ptr(), numLabels, numNeurons);
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                b1(bias2->c_ptr(), numLabels, 1);

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                y1(output2->c_ptr(), numLabels, sizeBatch);

            y1 = w1 * y0;
            y1 = y1.colwise() + b1.col(0);
            y1 = y1.unaryExpr(&sigmoid);
            y1 = y1.unaryExpr(&outLabel);

            // convert result to FFMatrixBlock
            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex,
                                               numLabels, sizeBatch, false);

            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                resultMatrix(outData, numLabels, sizeBatch);

            resultMatrix = y1;

            return resultFFMatrixBlock;
        });
    }
};

#endif
