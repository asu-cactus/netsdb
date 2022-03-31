#ifndef SEMANTIC_CLASSIFIER_H
#define SEMANTIC_CLASSIFIER_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include <eigen3/Eigen/Dense>

using namespace pdb;

double sigmoid(double x);
double relu(double x);
double outLabel(double x);

class SemanticClassifier : public SelectionComp<FFMatrixBlock, FFMatrixBlock> {
  private:
    uint32_t sizeEmbed;
    uint32_t sizeDense0;
    uint32_t sizeDense1;

  public:
    ENABLE_DEEP_COPY

    SemanticClassifier() {
        this->sizeEmbed = 500;
        this->sizeDense0 = 16;
        this->sizeDense1 = 1;
    }

    SemanticClassifier(uint32_t sizeEmbed, uint32_t sizeDense0, uint32_t sizeDense1) {
        this->sizeEmbed = sizeEmbed;
        this->sizeDense0 = sizeDense0;
        this->sizeDense1 = sizeDense1;
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
            // testing purpose
            // std::cout << inNumRow << "," << inNumCol << std::endl;
            // std::cout << inBlockRowIndex << "," << inBlockColIndex << std::endl;

            uint32_t sizeEmbed = this->sizeEmbed;
            uint32_t sizeBatch = inNumRow;
            uint32_t sizeDense0 = this->sizeDense0;
            uint32_t sizeDense1 = this->sizeDense1;

            std::cout << "Model Structure: " << sizeEmbed << "," << sizeDense0 << "," << sizeDense1 << std::endl;
            // std::cout << inBlockRowIndex << "," << inBlockColIndex << std::endl;


            // init weights and bias
            std::vector<double> embedOutput(sizeEmbed * sizeBatch, 1);
            std::vector<double> dense0Weight(sizeDense0 * sizeEmbed, 1);
            std::vector<double> dense0Bias(sizeDense0, 1);
            std::vector<double> dense0Output(sizeDense0 * sizeBatch, 1);
            std::vector<double> dense1Weight(sizeDense1 * sizeDense0, 1);
            std::vector<double> dense1Bias(sizeDense1, 1);
            std::vector<double> dense1Output(sizeDense1 * sizeBatch, 1);
            
            double *inData = in->getValue().rawData->c_ptr();
            // dense 0
            // x0 [sizeEmbed, sizeBatch]
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                x0(inData, sizeEmbed, sizeBatch);
            // w0 [sizeDense0, sizeEmbed]
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                w0(&dense0Weight.data()[0], sizeDense0, sizeEmbed);
            // b1 [sizeDense0, 1]
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                b0(&dense0Bias.data()[0], sizeDense0, 1);
            // y0
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                y0(&dense0Output.data()[0], sizeDense0, sizeBatch);

            // computation for the dense 0
            y0 = w0 * x0;
            y0 = y0.colwise() + b0.col(0);
            y0 = y0.unaryExpr(&relu);

            // dense 1
            // x1 = y0
            // w1 [sizeDense1, sizeDense0]
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                w1(&dense1Weight.data()[0], sizeDense1, sizeDense0);
            // b1 [sizeDense1, 1]
            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                b1(&dense1Bias.data()[0], sizeDense1, 1);

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                y1(&dense1Output.data()[0], sizeDense1, sizeBatch);

            // computation for the dense 1
            y1 = w1 * y0;
            y1 = y1.colwise() + b1.col(0);
            y1 = y1.unaryExpr(&sigmoid);
            y1 = y1.unaryExpr(&outLabel);

            // convert result to FFMatrixBlock
            pdb::Handle<FFMatrixBlock> resultFFMatrixBlock =
                pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex,
                                               sizeDense1, sizeBatch, false);

            double *outData = resultFFMatrixBlock->getValue().rawData->c_ptr();

            Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                                     Eigen::RowMajor>>
                resultMatrix(outData, sizeDense1, sizeBatch);

            resultMatrix = y1;
            // testing purpose
            /* std::cout << "===========Product result ==========" << std::endl;
            std::cout << resultMatrix.rows() << "," << resultMatrix.cols()
                      << std::endl;
            std::cout << resultFFMatrixBlock->getRowNums() << ","
                      << resultFFMatrixBlock->getColNums() << std::endl;
            std::cout << y1.rows() << "," << y1.cols() << std::endl; */
            // TODO Temp added for debugging
            std::cout << "output shape: " << y1.rows() << "," << y1.cols() << std::endl;
            return resultFFMatrixBlock;
        });
    }
};

#endif
