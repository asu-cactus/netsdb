#include <mkl_vsl.h>
#include <mkl.h>

#include <PDBClient.h>
#include <MatrixSparse.h>


void storeMatrix(PDBClient &pdbClient,
                 const std::string &db, const std::string &set,
                 size_t numRows, size_t numCols,
                 int blockRowNums,int blockColNums, float p,
                  int memoryBlockSize) {

  string errMsg;

  VSLStreamStatePtr rng;

  vslNewStream(&rng, VSL_BRNG_MT19937, time(nullptr));
 int ii=0;
 int jj=0;


  auto expectedNNZ = (int)(blockRowNums * std::min(p * 1.5f, 1.0f));

  std::vector<MKL_INT> rows(expectedNNZ);
  std::vector<MKL_INT> cols(expectedNNZ);

  std::vector<float> colProbs(blockColNums);

  size_t stripID = 0;
 while(true){ 
      
	pdb::makeObjectAllocatorBlock(memoryBlockSize * 1024 * 1024, true);

      pdb::Handle<pdb::Vector<pdb::Handle<MatrixSparse>>> storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<MatrixSparse>>>();
	 std::cout<<"in while /n";
            std::cout<<"in while /n";


      try {

	for(ii=ii;ii<numRows;ii++) {
       for(jj=jj;jj<numCols;jj++){
        MKL_INT nnz = 0;     
    pdb::Handle<MatrixSparse> myData = pdb::makeObject<MatrixSparse>(ii,jj, blockRowNums, blockColNums);

        vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, rng, blockColNums, colProbs.data(), 0, 1);

        for(int i = 0; i < blockRowNums; i++) {


          for(int j = 0; j < blockColNums; j++) {

            if(colProbs[j] >= p) {

              rows.emplace_back(i);
              cols.emplace_back(j);
              nnz++;
            }
          }
        }

        rows.resize(nnz);
        cols.resize(nnz);

        std::vector<float> vals(nnz);
        vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, rng, nnz, vals.data(), -1000, 1000);

        sparse_matrix_t cooMatrix;
        mkl_sparse_s_create_coo(&cooMatrix, SPARSE_INDEX_BASE_ZERO, blockRowNums, blockColNums, nnz, rows.data(), cols.data(), vals.data());

        sparse_matrix_t csrMatrix;
        mkl_sparse_convert_csr(cooMatrix, SPARSE_OPERATION_NON_TRANSPOSE, &csrMatrix);

        sparse_index_base_t indexing;
        MKL_INT outRows;
        MKL_INT outCols;
        MKL_INT *rows_start;
        MKL_INT *rows_end;
        MKL_INT *col_indx;
        float *values;

        // get all the stuff out
        mkl_sparse_s_export_csr(csrMatrix, &indexing, &outRows, &outCols, &rows_start, &rows_end, &col_indx, &values);
        nnz = rows_end[outRows - 1] - rows_start[0];
        memcpy(myData->rowIndices.c_ptr(), rows_start, sizeof(MKL_INT) * nnz);
       myData->rowIndices.c_ptr()[outRows] = rows_end[outRows-1];
        memcpy(myData->colIndices.c_ptr(), col_indx, sizeof(MKL_INT) * nnz);
        memcpy(myData->values.c_ptr(), values,sizeof(float)*nnz);
        // destroy the matrix
        mkl_sparse_destroy(csrMatrix);
        mkl_sparse_destroy(cooMatrix);

        // add the data to the vector
        storeMe->push_back(myData);
 
         }

	}
	if(!pdbClient.sendData<MatrixSparse>(std::pair<std::string, std::string>(set, db), storeMe, errMsg)){

	std::cout<<"Failed to send data" <<std::endl;
	exit(1);
	}

	break;


       }
    
       catch (pdb::NotEnoughSpace& n) {

        // mark as sent
        //sent = true;

        // send the data
       if(! pdbClient.sendData<MatrixSparse>(std::pair<std::string, std::string>(set, db), storeMe, errMsg)){
         
	  std:cout<<"failed to send data"<<std::endl;
        exit(1);
       }
       // std:cout<<"failed to send data"<<std::endl;
        //exit(1);   
        
      }

      // if it was not send through an exception send it
     // if(!sent) {

      //   send the data
       // pdbClient.sendData<MatrixBlock>(std::pair<std::string, std::string>(set, db), storeMe, errMsg);
      //}
        
   
   }  


  // free the random number generator
  vslDeleteStream(&rng);

  // to write back all buffered records
  pdbClient.flushData(errMsg);
}

int main(int argc, char* argv[]) {

  // the error is put here if something goes wrong
  string errMsg;

  if(argc != 10) {

    std::cout << "Usage : ./LoadSparseMatrices managerIP managerPort N M K S1 S2 p1 p2\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "N M K - We as generating two matrices of size NxM and MxK\n";
    std::cout << "S1 S2 - The number of strips we split the N and K dimension into respectively.\n";
    std::cout << "p1 p2 - The probabilities that a value in a matrix is sparse, p1 for the NxM, p2 for the MxK\n";
  }

  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);

  uint64_t n = std::stoi(argv[3]);
  uint64_t m = std::stoi(argv[4]);
  uint64_t k = std::stoi(argv[5]);
  int blockRowNums =std::stoi(argv[6]);
  int blockColNums =std::stoi(argv[7]);

  char *end;
  float p1 = std::stof(argv[8]);
  float p2 = std::stof(argv[9]);

  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  CatalogClient catalogClient(port, ip, clientLogger);

  pdbClient.registerType("libraries/libMatrixSparse.so", errMsg);
   pdbClient.registerType("libraries/libMatrixMeta.so", errMsg);
  pdbClient.registerType("libraries/libMatrixData.so", errMsg);


  pdbClient.createDatabase("db", errMsg);

 if(!pdbClient.createSet<MatrixSparse>("db", "a",errMsg,(size_t)64*(size_t)1024*(size_t)1024,"LoadLeftMatrixforMultiply")){

cout<<"not able to create set";
exit(-1);
}
else{
cout<<"created set \n";
}

 if(!pdbClient.createSet<MatrixSparse>("db", "b",errMsg,(size_t)64*(size_t)1024*(size_t)1024,"LoadrightMatrixforMultiply")){

cout<<"not able to create set";
exit(-1);
}
else{
cout<<"created set \n";
}

 storeMatrix(pdbClient, "db", "a", n, m, blockRowNums,blockColNums ,p1, 64);
 storeMatrix(pdbClient, "db", "b", m, k, blockRowNums,blockColNums, p2,  64);
}
