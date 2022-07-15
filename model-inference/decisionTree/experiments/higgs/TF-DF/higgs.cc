/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Usage example:
//  ./compile_and_run.sh
//

#include <memory>
#include <vector>

#include "absl/flags/flag.h"
#include "yggdrasil_decision_forests/dataset/data_spec.h"
#include "yggdrasil_decision_forests/dataset/data_spec.pb.h"
#include "yggdrasil_decision_forests/dataset/data_spec_inference.h"
#include "yggdrasil_decision_forests/dataset/vertical_dataset_io.h"
#include "yggdrasil_decision_forests/metric/metric.h"
#include "yggdrasil_decision_forests/metric/report.h"
#include "yggdrasil_decision_forests/model/abstract_model.h"
#include "yggdrasil_decision_forests/model/model_library.h"
#include "yggdrasil_decision_forests/model/prediction.pb.h"
#include "yggdrasil_decision_forests/utils/filesystem.h"
#include "yggdrasil_decision_forests/utils/logging.h"

ABSL_FLAG(std::string, dataset_dir, "yggdrasil_decision_forests/test_data/dataset", "Data directory.");

ABSL_FLAG(std::string, model_dir, "yggdrasil_decision_forests/test_data/model", "Model directory.");

ABSL_FLAG(int, num_records_by_shard_in_output, -1,
          "Number of records per output shards. Only valid if the output "
          "path is sharded (e.g. contains @10).");

ABSL_FLAG(std::string, output_dir, "/tmp/yggdrasil_decision_forest",
          "Output directory for the model and evaluation");

namespace ygg = yggdrasil_decision_forests;

int main(int argc, char** argv) {

  auto begin = std::chrono::high_resolution_clock::now();
  // Enable the logging. Optional in most cases.
  InitLogging(argv[0], &argc, &argv, true);

  // Check required flags.
  QCHECK(!absl::GetFlag(FLAGS_dataset_dir).empty());
  QCHECK(!absl::GetFlag(FLAGS_output_dir).empty());
  QCHECK(!absl::GetFlag(FLAGS_model_dir).empty());

  // Path to the testing dataset.

  const auto test_dataset_path = absl::StrCat(
      "csv:",
      file::JoinPath(absl::GetFlag(FLAGS_dataset_dir), "HIGGS.csv_test.csv"));

  // Create the output directory
  QCHECK_OK(file::RecursivelyCreateDir(absl::GetFlag(FLAGS_output_dir),
                                       file::Defaults()));

  // Load the model.
  LOG(INFO) << "Load model";

  const auto model_path =
      file::JoinPath(absl::GetFlag(FLAGS_model_dir), "higgs_binary_class_rf/assets");

  // Load the model
  std::unique_ptr<ygg::model::AbstractModel> model;
  QCHECK_OK(ygg::model::LoadModel(model_path, &model));

  // Show information about the model.
  // Like :show_model, but without the list of compatible engines.
  std::string model_description;
  model->AppendDescriptionAndStatistics(/*full_definition=*/false,
                                        &model_description);
  QCHECK_OK(
      file::SetContent(absl::StrCat(model_path, ".txt"), model_description));
  LOG(INFO) << "Model:\n" << model_description;
   

  // Evaluate the model
  // Same as :evaluate
  ygg::dataset::VerticalDataset test_dataset;
  QCHECK_OK(ygg::dataset::LoadVerticalDataset(
      test_dataset_path, model->data_spec(), &test_dataset));

  // Compute the predictions.
  std::vector<ygg::model::proto::Prediction> predictions;
  predictions.resize(test_dataset.nrow());

  // Compile the model for fast inference.
  const std::unique_ptr<ygg::serving::FastEngine> serving_engine =
      model->BuildFastEngine().value();

  // Allocate examples.
  std::unique_ptr<ygg::serving::AbstractExampleSet> examples =
      serving_engine->AllocateExamples(test_dataset.nrow());

  QCHECK_OK(ygg::serving::CopyVerticalDatasetToAbstractExampleSet(
        test_dataset, 0, test_dataset.nrow(), serving_engine->features(), examples.get()));


  std::vector<float> fast_predictions;

  // Run the predictions.
  serving_engine->Predict(*examples, test_dataset.nrow(), &fast_predictions);
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "End-to-End Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  return 0;
}
