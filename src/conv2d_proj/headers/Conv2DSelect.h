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

class Conv2DSelect: public SelectionComp<pdb::TensorData, pdb::TensorData>{

public:

    ENABLE_DEEP_COPY

    Conv2DSelect() {}

    Conv2DSelect(Handle<TensorData> filters, std::string convMode = "aten-conv2d", int stride = 1) {

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

        //set up the mode of the convolutional operation
        this->conv2dMode = convMode;

        this->stride = stride;

    }

    Lambda<bool> getSelection(Handle<TensorData> checkMe) override {

        return makeLambda(checkMe, [](Handle<TensorData>& checkMe) { return true; });

    }


    Handle<TensorData> runEigenSpatial(TensorData& input,  int n, int z, int y, int x, int stride) {

        std::cout << "---------------------------runEigenSpatial-------------------------:" << n << " " << z << " "<< y << " "<< x << " " << stride << std::endl;
        Eigen::TensorMap<Eigen::Tensor<float, 4>> a (input.rawData->c_ptr(), n, z, y, x);
        
        //Eigen::Tensor<float, 3> c = a.convolve(b1)

        Eigen::TensorMap<Eigen::Tensor<float, 4>> b1(kernel->rawData->c_ptr(), nk, zk, yk, xk);


        //NumDims of input: 3
        //kernelFilters: nk
        //kernelChannels: zk
        //kernelRows: yk
        //kernelCols: xk
        //kernelRowsEff = kernelRows + (kernelRows-1)*(row_in_stride-1) = yk
        //kernelColsEff = kernelCols + (kernelCols-1)*(col_in_stride-1) = xk
        

        //contract_dims
        Eigen::array<Eigen::IndexPair<int>, 1> contract_dims;
        contract_dims[0] = Eigen::IndexPair<int>(1, 0);


        //out_height = (inputRows - kernelRowsEff) / stride + 1 = (y - yk) / stride + 1
        //out_width = (inputCols - kernelColsEff) / stride  + 1 = (x - xk) / stride + 1

       int oy = calculateOutputDimension(y, yk, stride);
       int ox = calculateOutputDimension(x, xk, stride);
       
        //pre_contract_dims
        Eigen::array<int, 2> pre_contract_dims;
        pre_contract_dims[0] = zk * yk * xk;
        pre_contract_dims[1] = (oy) * (ox) * x;

        //post_contract_dims
        Eigen::array<int, 3> post_contract_dims;
        post_contract_dims[0] = nk;
        post_contract_dims[1] = (oy);
        post_contract_dims[2] = (ox) * x;

        //kernel dims
        Eigen::array<int, 2> kernel_dims;
        kernel_dims[0] = nk;
        kernel_dims[1] = zk * yk * xk;


        //create the output
        
        Handle<Vector<unsigned int>> dimensions = makeObject<Vector<unsigned int>>(4);
        dimensions->push_back(n);

        dimensions->push_back(nk);
        
        dimensions->push_back(oy);
        
        dimensions->push_back(ox);

        Handle<TensorData> out = makeObject<TensorData>(4, dimensions);

       float * mempool = (float *) malloc (n * nk * oy * ox * sizeof(float));

        Eigen::TensorMap<Eigen::Tensor<float, 4>> c (mempool, n, nk, oy, ox); 

        c = b1.reshape(kernel_dims)
              .contract(
                 a.extract_image_patches(yk, xk, stride, stride, 1, 1, 
                                             Eigen::PADDING_VALID)
                  .reshape(pre_contract_dims), contract_dims)
              .reshape(post_contract_dims);


        /* 
        c = a.extract_image_patches(yk, xk, 1, 1, 1, 1, Eigen::PADDING_VALID)

                                     .reshape(Eigen::array<int, 2>({(y - yk + 1 ) * (x - xk + 1 ), zk * yk * xk}))
                                     .contract(b1.reshape(Eigen::array<int, 2>({yk * xk, nk})), contract_dims)
                                     .reshape(Eigen::array<int, 3>({ (x - xk + 1 ), (y - yk + 1 ), nk }));

        */

       memcpy (out->rawData->c_ptr(), mempool, n * nk * oy * ox * sizeof(float));

       return out;
    }

    Handle<TensorData> runAtenConv2d(TensorData& input, int n, int z, int y, int x, int stride) {

        std::cout << "---------------------------RunAtendConv2d-------------------------:" << n << " " << z << " "<< y << " "<< x << " " << stride << std::endl;
        //input data
        // std::cout << "---------------------------input size ----------------------------:" << input.size << std::endl;
        at::Tensor a = at::from_blob(input.rawData->c_ptr(), {n, z, y, x});

        at::Tensor b = at::from_blob(kernel->rawData->c_ptr(), {nk, zk, yk, xk});

        // bias length = kernel count = nk
        at::Tensor bias = at::zeros({nk}, at::kFloat);
        //perform the convolutional operation
        auto begin = std::chrono::high_resolution_clock::now();
        // auto c = at::conv2d(a, b);
        auto c = at::conv2d(a, b, bias, stride);
        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "-------------------------------------------------------------------------Inside RunAten Time Duration: "
            << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
            << " secs." << std::endl;

        //create the output
        int oy = calculateOutputDimension(y, yk, stride);
        int ox = calculateOutputDimension(x, xk, stride);
        Handle<Vector<unsigned int>> dimensions = makeObject<Vector<unsigned int>>(4);

        dimensions->push_back(n);

        dimensions->push_back(nk);

        dimensions->push_back(oy);

        dimensions->push_back(ox);

        Handle<TensorData> out = makeObject<TensorData>(4, dimensions);
        memcpy(out->rawData->c_ptr(), c.storage().data(), n * nk * (oy) * (ox) * sizeof(float));
        
        return out;
    }



    Lambda<Handle<TensorData>> getProjection(Handle<TensorData> checkMe) override {

        return makeLambda(checkMe, [&](Handle<TensorData>& checkMe) {


            TensorData input = *checkMe;

            assert (input.numRanks = 4);

            //set up input dimensions

            //N
            int n = (*(input.dimensions))[0];

            //C
            int z = (*(input.dimensions))[1];

            //H
            int y = (*(input.dimensions))[2];

            //W
            int x = (*(input.dimensions))[3];

            std::cout << "------------------------------------------------------------------Before calling runAtenConv2d--------------------------------------------------" << std::endl;
            std::cout << "n =" << n << "z =" << z << "y =" << y << "x =" << x << std::endl;

            if (conv2dMode == "eigen-spatial") {
                return runEigenSpatial(input, n, z, y, x, stride);
            } else {
                return runAtenConv2d(input, n, z, y, x, stride);
            }
        });
    }

private:

    //multiple 2D-filters, each filter has many channels
    Handle<TensorData> kernel = nullptr;

    //conv2d-mode:
    //--"eigen-spatial": https://github.com/tensorflow/tensorflow/blob/v1.13.1/tensorflow/core/kernels/eigen_spatial_convolutions.h#L1688
    //https://github.com/pytorch/pytorch/blob/master/caffe2/operators/conv_op_eigen.cc
    //--"aten-conv2d": https://github.com/pytorch/pytorch/blob/master/aten/src/ATen/native/Convolution.cpp
    String conv2dMode = "aten-conv2d";


    int nk;

    int xk;
 
    int yk;

    int zk;

    unsigned int stride;

    static int calculateOutputDimension(int inputDimention, int filterDimention, int stride) {
        return (inputDimention - filterDimention) / stride + 1;
    }

};

#endif
