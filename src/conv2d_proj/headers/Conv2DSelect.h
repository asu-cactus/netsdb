#ifndef CONV2DSELECT_H
#define CONV2DSELECT_H
  
#include "SelectionComp.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "TensorData.h"

//LA libraries:
#include <eigen3/unsupported/Eigen/CXX11/Tensor>
#include <cmath>

//ATen libraries:
#include <ATen/ATen.h>
#include <ATen/Functions.h>

using namespace pdb;

//stride = 1

class Conv2DSelect: public SelectionComp<TensorData, TensorData> {

public:
    ENABLE_DEEP_COPY

    Conv2DSelect() {}

    Conv2DSelect(Handle<TensorData> filters, Handle<Vector<double>> biasVec, std::string convMode) {

        //make sure it's a 3D tensor
        assert(filters->numRanks = 4);

        //set up the kernel and kernel dimensions
        this->kernel = filters;

        //kernel count
        this->nk = (*(filters->dimensions))[0];

        //C
        this->zk = (*(filters->dimensions))[1];
         
        //H
        this->yk = (*(filters->dimensions))[2];

        //W
        this->xk = (*(filters->dimensions))[3];

        //set up the bias vector
        this->bias = biasVec;

        //set up the mode of the convolutional operation
        this->conv2dMode = convMode;

        if(this->conv2dMode == "aten-cov2d") {

           //setup the ATen kernel tensor for this class
           this->b = at::from_blob(kernel->rawData->c_ptr(), {nk, zk, yk, xk});

        } else if (this->conv2dMode == "eigen-spatial") {

           Eigen::TensorMap<Eigen::Tensor<float, 4>> b1(kernel->rawData->c_ptr(), nk, zk, yk, xk);

        }

    }

    Lambda<bool> getSelection(Handle<TensorData> checkMe) override {

        return makeLambda(checkMe, [](Handle<TensorData>& checkMe) { return true; });

    }


    Handle<TensorData> runEigenSpatial(TensorData& input, int z, int y, int x) {

        Eigen::TensorMap<Eigen::Tensor<float, 3>> a (input.rawData->c_ptr(), z, y, x);
        
        //Eigen::Tensor<float, 3> c = a.convolve(b1)


        //contract_dims
        Eigen::array<Eigen::IndexPair<int>, 1> contract_dims;
        contract_dims[0] = Eigen::IndexPair<int>(1, 0);

        /*

        //pre_contract_dims
        Eigen::DSizes<at::indexing::TensorIndex, 2> pre_contract_dims;
        pre_contract_dims[1] = zk * yk * xk;
        pre_contract_dims[0] = (y - yk + 1 ) * (x - xk + 1 )*z*y*x;

        //post_contract_dims
        Eigen::DSizes<at::indexing::TensorIndex, 3> post_contract_dims;
        post_contract_dims[0] = nk;
        post_contract_dims[1] = (y - yk + 1 );
        post_contract_dims[2] = (x - xk + 1 );

        //kernel dims
        Eigen::DSizes<at::indexing::TensorIndex, 2> kernel_dims;
        kernel_dims[0] = zk * yk * xk;
        kernel_dims[1] = nk;

*/
        //create the output
        
        Handle<Vector<unsigned int>> dimensions = makeObject<Vector<unsigned int>>(3);
        
        dimensions->push_back(nk);
        
        dimensions->push_back(y - yk + 1);
        
        dimensions->push_back(x - xk + 1);

        Handle<TensorData> out = makeObject<TensorData>(3, dimensions);

        

        Eigen::TensorMap<Eigen::Tensor<float, 3>> c(out->rawData->c_ptr(), (x - xk + 1 ), (y - yk + 1 ), nk);

        c = a.extract_image_patches(xk, yk, 1, 1, 1, 1, Eigen::PADDING_SAME)
                                     .reshape(Eigen::array<int, 2>({(y - yk + 1 ) * (x - xk + 1 )*z*y*x, zk * yk * xk}))
                                     .contract(b1.reshape(Eigen::array<int, 2>({zk * yk * xk, nk})), contract_dims)
                                     .reshape(Eigen::array<int, 3>({ (x - xk + 1 ), (y - yk + 1 ), nk }));


        return out;


    }

    Handle<TensorData> runAtenConv2d(TensorData& input, int z, int y, int x) {

        //input data
        at::Tensor a = at::from_blob(input.rawData->c_ptr(), {1, z, y, x});

        //perform the convolutional operation
        auto c = at::conv2d(a, b);

        //create the output

        Handle<Vector<unsigned int>> dimensions = makeObject<Vector<unsigned int>>(3);

        dimensions->push_back(nk);

        dimensions->push_back(y - yk + 1);

        dimensions->push_back(x - xk + 1);

        Handle<TensorData> out = makeObject<TensorData>(3, dimensions);

        memcpy(out->rawData->c_ptr(), c.storage().data(), nk * (y - yk + 1 ) * (x - xk + 1 ) * sizeof(float));

        return out;

    }



    Lambda<Handle<TensorData>> getProjection(Handle<TensorData> checkMe) override {

        return makeLambda(checkMe, [&](Handle<TensorData>& checkMe) {

            TensorData input = *checkMe;

            assert (input.numRanks = 3);

            //set up input dimensions

            //C
            int z = (*(input.dimensions))[0];

            //H
            int y = (*(input.dimensions))[1];

            //W
            int x = (*(input.dimensions))[2];


            if (conv2dMode == "eigen-spatial")

                  return runEigenSpatial(input, z, y, x);

            else 

                  return runAtenConv2d(input, z, y, x);

        });
    }

private:

    //multiple 2D-filters, each filter has many channels
    Handle<TensorData> kernel = nullptr;

    //bias vector
    Handle<Vector<double>> bias = nullptr;

    //conv2d-mode:
    //--"eigen-spatial": https://github.com/tensorflow/tensorflow/blob/v1.13.1/tensorflow/core/kernels/eigen_spatial_convolutions.h#L1688
    //https://github.com/pytorch/pytorch/blob/master/caffe2/operators/conv_op_eigen.cc
    //--"aten-conv2d": https://github.com/pytorch/pytorch/blob/master/aten/src/ATen/native/Convolution.cpp
    String conv2dMode = "aten-conv2d";

    //kernel for aten-cov2d mode
    at::Tensor b;

    //kernel for eigen-spatial mode
    Eigen::Tensor<float, 4> b1;

    int nk;

    int xk;
 
    int yk;

    int zk;


};

#endif
