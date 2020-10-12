#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>

#include "PDBClient.h"
#include "MatrixBlock.h"

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

bool read_label(ifstream &is, char *buffer, int32_t batch_id)
{

  int idx = 0;
  do
  {

    is.read(&buffer[idx], 1);
    if (buffer[idx] == ' ' || buffer[idx] == ',')
    {

      // add the end
      buffer[idx + 1] = '\0';

      // read the label
      char *tmp;
      auto label_id = strtol(buffer, &tmp, 10);

      // get the row id and col id
      auto row_id = batch_id / batch_block;
      auto col_id = label_id / labels_block;

      // store the labels
      labels[row_id * (num_labels / labels_block) + col_id].emplace_back(make_pair(batch_id % batch_block,
                                                                                   label_id % labels_block));

      // does not have other labels
      return buffer[idx] == ',';
    }

    // increment the index
    idx++;

  } while (true);
}

void read_feature_idx(ifstream &is, char *buffer, int32_t pos)
{

  int idx = 0;
  do
  {

    is.read(&buffer[idx], 1);
    if (buffer[idx] == ':')
    {

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

bool read_feature_value(ifstream &is, char *buffer, int32_t pos)
{

  int idx = 0;
  do
  {

    is.read(&buffer[idx], 1);
    if (buffer[idx] == ' ' || buffer[idx] == '\n')
    {

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

void init_features(int32_t row_id, int32_t col_id, double *values)
{

  auto start_r = row_id * batch_block;
  auto end_r = (row_id + 1) * batch_block;

  for (int r = start_r; r < end_r; ++r)
  {

    // get the features row
    auto &features_row = features[r];

    // get the start and end column
    auto start_c = col_id * features_block;
    auto end_c = (col_id + 1) * features_block;

    for (const auto &f : features_row)
    {
      if (f.first >= start_c && f.first < end_c)
      {
        values[(r % batch_block) * features_block + (f.first % features_block)] = f.second;
      }
    }
  }
}

void send_features(pdb::PDBClient &pdbClient, vector<pair<int32_t, int32_t>> &tuples_to_send, string setName, string dbName)
{
  string errMsg;
  size_t idx = 0;
  while (idx != tuples_to_send.size())
  {

    // use temporary allocation block
    const pdb::UseTemporaryAllocationBlock tempBlock{64 * 1024 * 1024};

    // put the chunks here
    pdb::Handle<pdb::Vector<pdb::Handle<MatrixBlock>>> vec = pdb::makeObject<pdb::Vector<pdb::Handle<MatrixBlock>>>();

    try
    {

      // put stuff into the vector
      for (; idx < tuples_to_send.size();)
      {

        // allocate a matrix
        pdb::Handle<MatrixBlock> myInt = pdb::makeObject<MatrixBlock>(tuples_to_send[idx].first,
                                                                      tuples_to_send[idx].second,
                                                                      batch_block,
                                                                      features_block);

        // init the values
        double *vals = myInt->getValue().rawData->c_ptr();
        bzero(myInt->getValue().rawData->c_ptr(), batch_block * features_block * sizeof(double));

        init_features(tuples_to_send[idx].first, tuples_to_send[idx].second, myInt->getValue().rawData->c_ptr());

        // we add the matrix to the block
        vec->push_back(myInt);

        // go to the next one
        ++idx;

        if (vec->size() == 50)
        {
          break;
        }
      }
    }
    catch (pdb::NotEnoughSpace &n)
    {
    }

    // init the records
    pdb::getRecord(vec);

    // send the data a bunch of times
    if (!pdbClient.sendData<MatrixBlock>(pair<string, string>(setName, dbName), vec, errMsg)) {
      cout << "Failed to send data to dispatcher server: " << errMsg << endl;
      exit(1);
    }

    // log that we stored stuff
    cout << "stored in input batch " << vec->size() << " !" << endl;
  }
}

void load_input_data(pdb::PDBClient &pdbClient, string path, string dbName, string setName)
{
  if (path.size() == 0)
  {
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
  if ((num_features % features_block) != 0)
  {
    num_features += features_block - (num_features % features_block);
  }

  // round labels so we can pad them
  if ((num_labels % labels_block) != 0)
  {
    num_labels += labels_block - (num_labels % labels_block);
  }

  // check that we have enough data points
  if (total_points < num_batch)
  {
    throw runtime_error("Not enough data points to form a batch " + to_string(total_points));
  }

  // init the data
  labels.resize((num_batch / batch_block) * (num_labels / labels_block));
  features.resize(num_batch);

  // load a single batch into memory
  char buffer[1024];
  for (int i = 0; i < num_batch; ++i)
  {

    // read the labels
    while (read_label(is, buffer, i))
    {
    }

    do
    {

      // try to read the feature index
      read_feature_idx(is, buffer, i);

      // try to read the feature value
      if (!read_feature_value(is, buffer, i))
      {
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
  for (int i = 0; i < batch_s; ++i)
  {
    for (int j = 0; j < batch_f; ++j)
    {
      tuples_to_send[i * batch_f + j] = {i, j};
    }
  }

  send_features(pdbClient, tuples_to_send, setName, dbName);

  int32_t batch_l = num_labels / labels_block;
  labels_meta.resize(batch_s * batch_l);
  // figure out all the blocks we need to send
  for (int i = 0; i < batch_s; ++i)
  {
    for (int j = 0; j < batch_l; ++j)
    {

      //                                pos,                num
      labels_meta[i * batch_l + j] = {labels_data.size(), labels[i * batch_l + j].size()};

      // insert the labels data
      labels_data.insert(labels_data.end(), labels[i * batch_l + j].begin(), labels[i * batch_l + j].end());
    }
  }
}

int main(int argc, char *argv[])
{
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

  if (!pdbClient.createDatabase("ff", errMsg)) {
      cout << "Not able to create database: " << errMsg << endl;
      exit(-1);
  } else {
      cout << "Created database" << endl;
  }

  // now, create the first matrix set in that database
  if (!pdbClient.createSet<MatrixBlock>("ff", "input_batch", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "inputBatch")) {
      cout << "Not able to create set: " + errMsg;
      exit(-1);
  } else {
      cout << "Created set.\n";
  }

  // load the input data
  load_input_data(pdbClient, path, "ff", "input_batch");

  return 0;
}