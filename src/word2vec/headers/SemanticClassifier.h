#ifndef SEMANTIC_CLASSIFIER_H
#define SEMANTIC_CLASSIFIER_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include <eigen3/Eigen/Dense>

using namespace pdb;

class SemanticClassifier : public SelectionComp<FFMatrixBlock, FFMatrixBlock>
{
private:
    uint32_t sizeEmbed;
    uint32_t sizeBatch;
    uint32_t sizeDense0;
    uint32_t sizeDense1;
    std::vector<double> embedOutput;
    std::vector<double> dense0Weight, dense0Bias, dense0Output;
    std::vector<double> dense1Weight, dense1Bias, dense1Output;

public:
    ENABLE_DEEP_COPY

    SemanticClassifier()
    {
        // this->sizeEmbed = 500;
        // this->sizeBatch = 100;
        // this->sizeDense0 = 16;
        // this->sizeDense1 = 1;

        // this->embedOutput = std::vector<double>(sizeEmbed * sizeBatch, 1);
        // this->dense0Weight = std::vector<double>(sizeDense0 * sizeEmbed, 1);
        // this->dense0Bias = std::vector<double>(sizeDense0, 1);
        // this->dense0Output = std::vector<double>(sizeDense0 * sizeBatch, 1);
        // this->dense1Weight = std::vector<double>(sizeDense1 * sizeDense0, 1);
        // this->dense1Bias = std::vector<double>(sizeDense1, 1);
        // this->dense1Output = std::vector<double>(sizeDense1 * sizeBatch, 1);

        std::cout << "Initialized classifier successfully!  bbb" << std::endl;
    }

    Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override
    {
        return makeLambda(checkMe, [](Handle<FFMatrixBlock> &checkMe)
                          {
                              std::cout << "Initialized classifier successfully! Selection" << std::endl;
                              return true;
                          });
    }

    Lambda<Handle<FFMatrixBlock>> getProjection(Handle<FFMatrixBlock> checkMe) override
    {
        return makeLambda(checkMe, [this](Handle<FFMatrixBlock> &checkMe)
                          {
                              std::cout << "In Projection succ!" << std::endl;
                              std::cerr << "In Projection bbbbb!" << std::endl;

                              return checkMe;

                              /*

                                // dense 0
                                // x0 [sizeEmbed, sizeBatch]
                                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> x0
                                (&embedOutput.data()[0], sizeEmbed, sizeBatch);
                                // w0 [sizeDense0, sizeEmbed]
                                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> w0
                                (&dense0Weight.data()[0], sizeDense0, sizeEmbed);
                                // b1 [sizeDense0, 1]
                                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> b0
                                (&dense0Bias.data()[0], sizeDense0, 1);    

                                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> y0
                                (&dense0Output.data()[0], sizeDense0, sizeBatch);

                                // computation for the dense 0 
                                y0 = w0 * x0;
                                y0 = y0.colwise() + b0.col(0);
                                y0 = y0.unaryExpr(&relu);
                                
                                // dense 1
                                // x1 = y0
                                // w0 [sizeDense0, sizeEmbed]
                                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> w1
                                (&dense1Weight.data()[0], sizeDense1, sizeDense0);
                                // b1 [sizeDense0, 1]
                                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> b1
                                (&dense1Bias.data()[0], sizeDense1, 1);    

                                Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> y1
                                (&dense1Output.data()[0], sizeDense1, sizeBatch);

                                // computation for the dense 1
                                y1 = w1 * y0;
                                y1 = y1.colwise() + b1.col(0);
                                y1 = y1.unaryExpr(&sigmoid);
                                y1 = y1.unaryExpr(&outLabel);
                                // y0 += b0;


                              */
                          });
    }

    double sigmoid(double x)
    {
        return 1 / (1 + exp(-x));
    }

    double relu(double x)
    {
        return std::max(0.0, x);
    }

    double outLabel(double x)
    {
        double threshold = 0.5;
        if (x > threshold)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
};

#endif
