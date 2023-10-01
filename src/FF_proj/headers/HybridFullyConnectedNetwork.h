#ifndef HYBRID_FULLY_CONNECTED_NETWORK_H
#define HYBRID FULLY_CONNECTED_NETWORK_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include <eigen3/Eigen/Dense>


using namespace pdb;

float sigmoid1(float x);
float relu1(float x);
float outLabel1(float x);
void randomGen1(Handle<Vector<float>> vec);

class HybridFullyConnectedNetwork : public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  private:
    int numFeatures;
    int numLabels;
    int numNeurons;
    uint32_t sizeBatch;
    Handle<Vector<float>> bias1 = nullptr; //( numNeurons, 1);
    Handle<Vector<float>> output1 = nullptr; //( sizeBatch * numNeurons, 1);
    Handle<Vector<float>> weights2 = nullptr; //( numNeurons * numLabels, 1);
    Handle<Vector<float>> bias2 = nullptr; //( numLabels, 1);
    Handle<Vector<float>> output2 = nullptr; //( sizeBatch * numLabels, 1);

  public:
    ENABLE_DEEP_COPY

    HybridFullyConnectedNetwork() {
    }

    HybridFullyConnectedNetwork(int numFeatures, int numLabels, int numNeurons, uint32_t sizeBatch) {
        this->numFeatures = numFeatures;
	this->numLabels = numLabels;
	this->numNeurons = numNeurons;
	this->sizeBatch = sizeBatch;
	this->bias1 = makeObject<Vector<float>>(numNeurons, numNeurons);
	randomGen1(bias1);
	this->weights2 = makeObject<Vector<float>>(numNeurons * numLabels, numNeurons * numLabels);
	randomGen1(weights2);
	this->bias2 = makeObject<Vector<float>>(numLabels, numLabels);
	randomGen1(bias2);
    }

    ~HybridFullyConnectedNetwork() {
	this->bias1 = nullptr;
	this->weights2 = nullptr;
	this->bias2 = nullptr;
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
	        sizeBatch = inNumRow; 
	    }
            float * output1 = (float *) malloc (sizeBatch * numNeurons * sizeof(float));
            float *inData = in->getValue().rawData->c_ptr();
            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                b0(bias1->c_ptr(), numNeurons, 1);
            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                y0(inData, numNeurons, sizeBatch);

            y0 = y0.colwise() + b0.col(0);
            y0 = y0.unaryExpr(&relu1);

	     // convert result to FFMatrixBlock
            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex,
                                               numLabels, sizeBatch, false);

	    float *output2 = resultFFMatrixBlock->getValue().rawData->c_ptr();

            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                w1(weights2->c_ptr(), numLabels, numNeurons);
            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                b1(bias2->c_ptr(), numLabels, 1);

            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                y1(output2, numLabels, sizeBatch);

            y1 = w1 * y0;
            y1 = y1.colwise() + b1.col(0);
            y1 = y1.unaryExpr(&sigmoid1);
            y1 = y1.unaryExpr(&outLabel1);


	    delete output1;

            return resultFFMatrixBlock;
        });
    }
};

#endif
