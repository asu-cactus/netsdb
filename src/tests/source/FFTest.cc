#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "FFInputLayerJoin.h"
#include "FFMatrixBlock.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixWriter.h"
#include "FFAggMatrix.h"
#include "FFHiddenLayerJoin.h"
#include "FFSelectionGradient.h"
#include "FFJoinBackTransposeMult.h"
#include "PDBClient.h"

using namespace std;

vector<vector<pair<int32_t, int32_t>>> labels;

vector<pair<int32_t, int32_t>> labels_meta;
vector<pair<int32_t, int32_t>> labels_data;

vector<vector<pair<int32_t, float>>> features;

int32_t total_points;

int32_t num_batch;
int32_t batch_block;

int32_t num_features;
int32_t features_block;

int32_t num_labels;
int32_t labels_block;

int32_t embedding_size;
int32_t embedding_block;

bool read_label(ifstream &is, char *buffer, int32_t batch_id) {

  int idx = 0;
  do {

    is.read(&buffer[idx], 1);
    if (buffer[idx] == ' ' || buffer[idx] == ',') {

      // add the end
      buffer[idx + 1] = '\0';

      // read the label
      char *tmp;
      auto label_id = strtol(buffer, &tmp, 10);

      // get the row id and col id
      auto row_id = batch_id / batch_block;
      auto col_id = label_id / labels_block;

      // store the labels
      labels[row_id * (num_labels / labels_block) + col_id].emplace_back(
          make_pair(batch_id % batch_block, label_id % labels_block));

      // does not have other labels
      return buffer[idx] == ',';
    }

    // increment the index
    idx++;

  } while (true);
}

void read_feature_idx(ifstream &is, char *buffer, int32_t pos) {

  int idx = 0;
  do {

    is.read(&buffer[idx], 1);
    if (buffer[idx] == ':') {

      // add the end
      buffer[idx + 1] = '\0';

      // read the label
      char *tmp;
      features[pos].emplace_back();
      features[pos].back().first = strtol(buffer, &tmp, 10);

      // does not have other labels
      return;
    }

    // increment the index
    idx++;

  } while (true);
}

bool read_feature_value(ifstream &is, char *buffer, int32_t pos) {

  int idx = 0;
  do {

    is.read(&buffer[idx], 1);
    if (buffer[idx] == ' ' || buffer[idx] == '\n') {

      // add the end
      buffer[idx + 1] = '\0';

      // read the label
      string::size_type tmp;
      features[pos].back().second = stof(buffer, &tmp);

      // does not have other labels
      return buffer[idx] == ' ';
    }

    // increment the index
    idx++;

  } while (true);
}

void init_features(int32_t row_id, int32_t col_id, double *values) {

  auto start_r = row_id * batch_block;
  auto end_r = (row_id + 1) * batch_block;

  for (int r = start_r; r < end_r; ++r) {

    // get the features row
    auto &features_row = features[r];

    // get the start and end column
    auto start_c = col_id * features_block;
    auto end_c = (col_id + 1) * features_block;

    for (const auto &f : features_row) {
      if (f.first >= start_c && f.first < end_c) {
        values[(r % batch_block) * features_block +
               (f.first % features_block)] = f.second;
      }
    }
  }
}

void send_features(pdb::PDBClient &pdbClient,
                   vector<pair<int32_t, int32_t>> &tuples_to_send,
                   string setName, string dbName) {
  string errMsg;
  size_t idx = 0;
  while (idx != tuples_to_send.size()) {

    // use temporary allocation block
    const pdb::UseTemporaryAllocationBlock tempBlock{64 * 1024 * 1024};

    // put the chunks here
    pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> vec =
        pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

    try {

      // put stuff into the vector
      for (; idx < tuples_to_send.size();) {

        // allocate a matrix
        pdb::Handle<FFMatrixBlock> myInt = pdb::makeObject<FFMatrixBlock>(
            tuples_to_send[idx].first, tuples_to_send[idx].second, batch_block,
            features_block);

        // init the values
        double *vals = myInt->getValue().rawData->c_ptr();
        bzero(myInt->getValue().rawData->c_ptr(),
              batch_block * features_block * sizeof(double));

        init_features(tuples_to_send[idx].first, tuples_to_send[idx].second,
                      myInt->getValue().rawData->c_ptr());

        // we add the matrix to the block
        vec->push_back(myInt);

        // go to the next one
        ++idx;

        if (vec->size() == 50) {
          break;
        }
      }
    } catch (pdb::NotEnoughSpace &n) {
    }

    // init the records
    pdb::getRecord(vec);

    // send the data a bunch of times
    if (!pdbClient.sendData<FFMatrixBlock>(
            pair<string, string>(setName, dbName), vec, errMsg)) {
      cout << "Failed to send data to dispatcher server: " << errMsg << endl;
      exit(1);
    }

    // log that we stored stuff
    cout << "stored in input batch " << vec->size() << " !" << endl;
  }
}

void load_input_data(pdb::PDBClient &pdbClient, string path, string dbName,
                     string setName) {
  if (path.size() == 0) {
    throw runtime_error("Invalid filepath for features: " + path);
  }

  /// 1. Load the data from the file

  // open the input file
  ifstream is(path);

  // load the data stats
  is >> total_points;
  is >> num_features;
  is >> num_labels;

  cout << total_points << ", " << num_features << ", " << num_labels << endl;

  // round features so we can pad them
  if ((num_features % features_block) != 0) {
    num_features += features_block - (num_features % features_block);
  }

  // round labels so we can pad them
  if ((num_labels % labels_block) != 0) {
    num_labels += labels_block - (num_labels % labels_block);
  }

  // check that we have enough data points
  if (total_points < num_batch) {
    throw runtime_error("Not enough data points to form a batch " +
                        to_string(total_points));
  }

  // init the data
  labels.resize((num_batch / batch_block) * (num_labels / labels_block));
  features.resize(num_batch);

  // load a single batch into memory
  char buffer[1024];
  for (int i = 0; i < num_batch; ++i) {

    // read the labels
    while (read_label(is, buffer, i)) {
    }

    do {

      // try to read the feature index
      read_feature_idx(is, buffer, i);

      // try to read the feature value
      if (!read_feature_value(is, buffer, i)) {
        break;
      }

    } while (true);
  }

  /// 2. Resize data on block size

  // figure out how many blocks we need to generate
  int32_t batch_s = num_batch / batch_block;
  int32_t batch_f = num_features / features_block;

  // figure out all the blocks we need to send
  vector<pair<int32_t, int32_t>> tuples_to_send(batch_s * batch_f);
  for (int i = 0; i < batch_s; ++i) {
    for (int j = 0; j < batch_f; ++j) {
      tuples_to_send[i * batch_f + j] = {i, j};
    }
  }

  send_features(pdbClient, tuples_to_send, setName, dbName);

  int32_t batch_l = num_labels / labels_block;
  labels_meta.resize(batch_s * batch_l);
  // figure out all the blocks we need to send
  for (int i = 0; i < batch_s; ++i) {
    for (int j = 0; j < batch_l; ++j) {

      //                                pos,                num
      labels_meta[i * batch_l + j] = {labels_data.size(),
                                      labels[i * batch_l + j].size()};

      // insert the labels data
      labels_data.insert(labels_data.end(), labels[i * batch_l + j].begin(),
                         labels[i * batch_l + j].end());
    }
  }
}

auto init_weights(pdb::PDBClient &pdbClient) {
  string errMsg;

  /// 1. Init the word embedding a matrix of size (num_features x
  /// embedding_block)

  auto block_f = num_features / features_block;
  auto block_e = embedding_size / embedding_block;

  // the page size
  const int32_t page_size = 64;

  // all the block we need to send
  std::vector<std::pair<int32_t, int32_t>> tuples_to_send(block_f * block_e);
  for (int i = 0; i < block_f; ++i) {
    for (int j = 0; j < block_e; ++j) {
      tuples_to_send[i * block_e + j] = {i, j};
    }
  }

  size_t idx = 0;
  while (idx != tuples_to_send.size()) {

    // use temporary allocation block
    const pdb::UseTemporaryAllocationBlock tempBlock{page_size * 1024 * 1024};

    // put the chunks here
    pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> data =
        pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

    try {

      // put stuff into the vector
      for (; idx < tuples_to_send.size();) {

        // allocate a matrix
        pdb::Handle<FFMatrixBlock> myInt = pdb::makeObject<FFMatrixBlock>(
            tuples_to_send[idx].first, tuples_to_send[idx].second,
            features_block, embedding_block);

        // init the values
        double *vals = myInt->getValue().rawData->c_ptr();
        for (int v = 0; v < features_block * embedding_block; ++v) {
          vals[v] = (double)drand48() * 0.1;
        }

        // check if we need to init the bias here if so do it...
        if (tuples_to_send[idx].first == (block_f - 1)) {

          // allocate the bias if necessary
          myInt->getValue().bias = pdb::makeObject<pdb::Vector<double>>(
              embedding_block, embedding_block);

          // init the bias
          for (int i = 0; i < embedding_block; ++i) {
            myInt->getValue().bias->c_ptr()[i] = (double)drand48() * 0.1;
          }
        }

        // we add the matrix to the block
        data->push_back(myInt);

        // go to the next one
        ++idx;

        if (data->size() == 50) {
          break;
        }
      }
    } catch (pdb::NotEnoughSpace &n) {
    }

    // init the records
    pdb::getRecord(data);

    // send the data a bunch of times
    // pdbClient.sendData<FFMatrixBlock>("ff", "w1", data);
    if (!pdbClient.sendData<FFMatrixBlock>(pair<string, string>("w1", "ff"),
                                           data, errMsg)) {
      cout << "Failed to send data to dispatcher server: " << errMsg << endl;
      exit(1);
    }

    // log that we stored stuff
    std::cout << "stored in embedding " << data->size() << " !\n";
  }

  /// 2. Init the dense layer (embedding_block x block_l)

  // how many blocks we split the labels
  auto block_l = num_labels / labels_block;

  tuples_to_send.resize(block_e * block_l);
  for (int i = 0; i < block_e; ++i) {
    for (int j = 0; j < block_l; ++j) {
      tuples_to_send[i * block_l + j] = {i, j};
    }
  }

  idx = 0;
  while (idx != tuples_to_send.size()) {

    // use temporary allocation block
    const pdb::UseTemporaryAllocationBlock tempBlock{page_size * 1024 * 1024};

    // put the chunks here
    pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> data =
        pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

    try {

      // put stuff into the vector
      for (; idx < tuples_to_send.size();) {

        // allocate a matrix
        pdb::Handle<FFMatrixBlock> myInt = pdb::makeObject<FFMatrixBlock>(
            tuples_to_send[idx].first, tuples_to_send[idx].second,
            embedding_block, labels_block);

        // init the values
        double *vals = myInt->getValue().rawData->c_ptr();
        for (int v = 0; v < embedding_block * labels_block; ++v) {
          vals[v] = (double)drand48() * 0.1;
        }

        if (tuples_to_send[idx].first == (block_e - 1)) {

          // init the bias if necessary
          myInt->getValue().bias =
              makeObject<pdb::Vector<double>>(labels_block, labels_block);

          // init the bias
          for (int i = 0; i < labels_block; ++i) {
            myInt->getValue().bias->c_ptr()[i] = (double)drand48() * 0.1;
          }
        }

        // we add the matrix to the block
        data->push_back(myInt);

        // go to the next one
        ++idx;

        if (data->size() == 50) {
          break;
        }
      }
    } catch (pdb::NotEnoughSpace &n) {
    }

    // init the records
    pdb::getRecord(data);

    // send the data a bunch of times
    // pdbClient.sendData<ff::MatrixBlock>("ff", "w2", data);
    if (!pdbClient.sendData<FFMatrixBlock>(pair<string, string>("w2", "ff"),
                                           data, errMsg)) {
      cout << "Failed to send data to dispatcher server: " << errMsg << endl;
      exit(1);
    }

    // log that we stored stuff
    std::cout << "stored in dense " << data->size() << " !\n";
  }
}

int main(int argc, char *argv[]) {
  string errMsg;

  cout << "num_batch : " << endl;
  cin >> num_batch;

  cout << "batch_block : " << endl;
  cin >> batch_block;

  cout << "features_block : " << endl;
  cin >> features_block;

  cout << "labels_block : " << endl;
  cin >> labels_block;

  cout << "embedding_size : " << endl;
  cin >> embedding_size;

  cout << "embedding_block : " << endl;
  cin >> embedding_block;

  string path;
  cout << "Path to input: " << endl;
  cin >> path;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  pdbClient.registerType("libraries/libMatrixMeta.so", errMsg);
  pdbClient.registerType("libraries/libMatrixData.so", errMsg);
  pdbClient.registerType("libraries/libMatrixBlock.so", errMsg);
  pdbClient.registerType("libraries/libFFMatrixData.so", errMsg);
  pdbClient.registerType("libraries/libFFMatrixBlock.so", errMsg);
  pdbClient.registerType("libraries/libFFMatrixBlockScanner.so", errMsg);
  pdbClient.registerType("libraries/libFFInputLayerJoin.so", errMsg);
  pdbClient.registerType("libraries/libFFMatrixWriter.so", errMsg);
  pdbClient.registerType("libraries/libFFAggMatrix.so", errMsg);
  pdbClient.registerType("libraries/libFFHiddenLayerJoin.so", errMsg);
  pdbClient.registerType("libraries/libFFSelectionGradient.so", errMsg);
  pdbClient.registerType("libraries/libFFJoinBackTransposeMult.so", errMsg);

  if (!pdbClient.createDatabase("ff", errMsg)) {
    cout << "Not able to create database: " << errMsg << endl;
    exit(-1);
  } else {
    cout << "Created database" << endl;
  }

  // now, create the first matrix set in that database
  if (!pdbClient.createSet<FFMatrixBlock>(
          "ff", "input_batch", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024,
          "inputBatch")) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  // now, create the first matrix set in that database
  if (!pdbClient.createSet<FFMatrixBlock>(
          "ff", "w1", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024, "W1")) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  // now, create the first matrix set in that database
  if (!pdbClient.createSet<FFMatrixBlock>(
          "ff", "w2", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024, "W2")) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  // now, create the first matrix set in that database
  if (!pdbClient.createSet<FFMatrixBlock>(
          "ff", "activation_1", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024, "activation1")) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  // now, create the first matrix set in that database
  if (!pdbClient.createSet<FFMatrixBlock>(
          "ff", "gradient_2", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024, "gradient2")) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  // now, create the first matrix set in that database
  if (!pdbClient.createSet<FFMatrixBlock>(
          "ff", "d_w2", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024, "dW2")) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
  // now, create the first matrix set in that database
  if (!pdbClient.createSet<FFMatrixBlock>(
          "ff", "gradient_1_tmp", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024, "gradient1Tmp")) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }

  // load the input data
  load_input_data(pdbClient, path, "ff", "input_batch");
  init_weights(pdbClient);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "input_batch");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "w1");

    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFInputLayerJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation = pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("ff", "activation_1");
    myWriter->setInput(myAggregation);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    // do the activation of the first layer
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "activation_1");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "w2");

    // make the join
    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFHiddenLayerJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation = pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("ff", "activation_2");
    myWriter->setInput(myAggregation);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    // do the activation of the first layer
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> lhs =
        pdb::makeObject<FFMatrixBlockScanner>("ff", "activation_2");

    // make the join
    pdb::Handle<pdb::Computation> selection = pdb::makeObject<FFSelectionGradient>(num_batch / batch_block,
                                                                         num_labels / labels_block,
                                                                         labels_meta,
                                                                         labels_data);
    selection->setInput(lhs);

    // make the writer
    pdb::Handle<pdb::Computation> writer = pdb::makeObject<FFMatrixWriter>("ff", "gradient_2");
    writer->setInput(selection);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, writer)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  pdbClient.removeSet("ff", "activation_2", errMsg);

  {
    // do the activation of the first layer
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        pdb::makeObject<FFMatrixBlockScanner>("ff", "activation_1");
    pdb::Handle<pdb::Computation> readB =
        pdb::makeObject<FFMatrixBlockScanner>("ff", "gradient_2");

    // make the join
    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFJoinBackTransposeMult>(embedding_size / embedding_block);
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation = pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("ff", "d_w2");
    myWriter->setInput(myAggregation);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    // do the activation of the first layer
    const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        pdb::makeObject<FFMatrixBlockScanner>("ff", "gradient_2");
    pdb::Handle<pdb::Computation> readB =
        pdb::makeObject<FFMatrixBlockScanner>("ff", "w2");

    // make the join
    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFJoinBackTransposeMult>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation = pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("ff", "gradient_1_tmp");
    myWriter->setInput(myAggregation);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  sleep(20);

  return 0;
}