#ifndef FULLY_CONNECTED_NETWORK_H
#define FULLY_CONNECTED_NETWORK_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include <eigen3/Eigen/Dense>


using namespace pdb;

float sigmoid(float x);
float relu(float x);
float outLabel(float x);
void randomGen(Handle<Vector<float>> vec);

class FullyConnectedNetwork : public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  private:
    int numFeatures;
    int numLabels;
    int numNeurons;
    uint32_t sizeBatch;
    Handle<Vector<float>> weights1 = nullptr; //( numFeatures * numNeurons, 1);
    Handle<Vector<float>> bias1 = nullptr; //( numNeurons, 1);
    Handle<Vector<float>> output1 = nullptr; //( sizeBatch * numNeurons, 1);
    Handle<Vector<float>> weights2 = nullptr; //( numNeurons * numLabels, 1);
    Handle<Vector<float>> bias2 = nullptr; //( numLabels, 1);
    Handle<Vector<float>> output2 = nullptr; //( sizeBatch * numLabels, 1);

  public:
    ENABLE_DEEP_COPY

    FullyConnectedNetwork() {
    }

    FullyConnectedNetwork(int numFeatures, int numLabels, int numNeurons, uint32_t sizeBatch) {
        this->numFeatures = numFeatures;
	this->numLabels = numLabels;
	this->numNeurons = numNeurons;
	this->sizeBatch = sizeBatch;
	this->weights1 = makeObject<Vector<float>>(numFeatures*numNeurons, numFeatures*numNeurons);
        randomGen(weights1);
	this->bias1 = makeObject<Vector<float>>(numNeurons, numNeurons);
	randomGen(bias1);
	//this->output1 = makeObject<Vector<float>>(sizeBatch * numNeurons, sizeBatch * numNeurons);
	//randomGen(output1);
	this->weights2 = makeObject<Vector<float>>(numNeurons * numLabels, numNeurons * numLabels);
	randomGen(weights2);
	this->bias2 = makeObject<Vector<float>>(numLabels, numLabels);
	randomGen(bias2);
	//this->output2 = makeObject<Vector<float>>(sizeBatch * numLabels, sizeBatch * numLabels);
	//randomGen(output2);
    }

    ~FullyConnectedNetwork() {
        this->weights1 = nullptr;
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
                x0(inData, numFeatures, sizeBatch);
            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                w0(weights1->c_ptr(), numNeurons, numFeatures);
            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                b0(bias1->c_ptr(), numNeurons, 1);
            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                y0(output1, numNeurons, sizeBatch);

            y0 = w0 * x0;
            y0 = y0.colwise() + b0.col(0);
            y0 = y0.unaryExpr(&relu);

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
            y1 = y1.unaryExpr(&sigmoid);
            y1 = y1.unaryExpr(&outLabel);

            // convert result to FFMatrixBlock
            /*pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex,
                                               numLabels, sizeBatch, false);

            float *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();

            Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                resultMatrix(outData, numLabels, sizeBatch);

            resultMatrix = y1;*/

	    delete output1;

            return resultFFMatrixBlock;
        });
    }
};

#endif
