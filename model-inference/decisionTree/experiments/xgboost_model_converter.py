# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Utilities for converting XGBoost models into Tensorflow models."""

import json
import numpy as np
import os
import tempfile
from typing import Any, Dict, List, Optional, TypeVar, Union

from xgboost import XGBClassifier
import tensorflow as tf
import tensorflow_decision_forests as tfdf



def get_trees(xgboost_model):
    booster = xgboost_model.get_booster()
    # The json format was available in October 2017.
    # XGBoost 0.7 was the first version released with it.
    js_tree_list = booster.get_dump(with_stats=True, dump_format='json')
    js_trees = [json.loads(s) for s in js_tree_list]
    return js_trees


def convert(
    xgboost_model: XGBClassifier,
    intermediate_write_path: Optional[os.PathLike] = None,
) -> tf.keras.Model:
  """Converts a tree-based XGBoost model to a tensorflow model.
  Args:
    xgboost_model: the XGBoost model to be converted.
    intermediate_write_path: path to a directory. As part of the conversion
      process, a TFDF model is written to disk. If intermediate_write_path is
      specified, the TFDF model is written to this directory. Otherwise, a
      temporary directory is created that is immediately removed after this
      function executes. Note that in order to save the converted model and
      load it again later, this argument must be provided.

  Returns:
    a keras Model that emulates the provided XGBoost model.
  """
  if not intermediate_write_path:
    # No intermediate directory was provided, so this creates one using the
    # TemporaryDirectory context mananger, which handles teardown.
    intermediate_write_directory = tempfile.TemporaryDirectory()
    path = intermediate_write_directory.name
  else:
    # Uses the provided write path, and creates a null context manager as a
    # stand-in for TemporaryDirectory.
    intermediate_write_directory = contextlib.nullcontext()
    path = intermediate_write_path
  with intermediate_write_directory:
    tfdf_model = build_tfdf_model(xgboost_model, path)
  # The resultant tfdf model only receives the features that are used
  # to split samples in nodes in the trees as input. But we want to pass the
  # full design matrix as an input to match the scikit-learn API, thus we
  # create another tf.keras.Model with the desired call signature.
  template_input = tf.keras.Input(shape=(xgboost_model.n_features_in_,))
  # Extracts the indices of the features that are used by the TFDF model.
  # The features have names with the format "feature_<index-of-feature>".
  feature_names = tfdf_model.signatures[
      "serving_default"].structured_input_signature[1].keys()
  template_output = tfdf_model(
      {i: template_input[:, int(i.split("_")[1])] for i in feature_names})
  return tf.keras.Model(inputs=template_input, outputs=template_output)


def build_tfdf_model(
    xgboost_model: XGBClassifier,
    path: os.PathLike,
) -> tf.keras.Model:
  """Converts a XGBoost model into a TFDF model."""
  bias = 0.0

  gbt_builder = tfdf.builder.GradientBoostedTreeBuilder(
      path=path,
      objective=tfdf.py_tree.objective.ClassificationObjective(label="label",
          classes=[str(c) for c in xgboost_model.classes_],
      ),
      bias=bias,
  )

  js_trees = get_trees(xgboost_model)

  params = xgboost_model.get_xgb_params()

  print("XGBOOST_MODEL LEARNING RATE:")
  print(xgboost_model.learning_rate)

  for jstree in js_trees:
    gbt_builder.add_tree(convert_xgboost_tree_to_tfdf_pytree(
        jstree,
        xgboost_model.learning_rate,
    ))
  gbt_builder.close()
  return tf.keras.models.load_model(path)


def convert_xgboost_tree_to_tfdf_pytree(
    xgboost_tree: str,
    weight: Optional[float] =None,
) -> tfdf.py_tree.tree.Tree:
  """Converts a XGBoost decision tree into a TFDF pytree.

  Args:
    xgboost_tree: a XGBoost decision tree in JSON format.

  Returns:
    a TFDF pytree that has the same structure as the xgboost tree.
  """

  root_node = _convert_xgboost_node_to_tfdf_node(
      xgboost_tree,
      weight,
  )
  return tfdf.py_tree.tree.Tree(root_node)



def _convert_xgboost_node_to_tfdf_node(
    jsnode: str,
    weight: Optional[float] =None,
) -> tfdf.py_tree.node.AbstractNode:
  """Converts a node within a xgboost tree into a TFDF node."""

  if 'children' in jsnode:
      feature = tfdf.py_tree.dataspec.SimpleColumnSpec(
              name = f"feature_{jsnode['split'][1:]}",
              type = tfdf.py_tree.dataspec.ColumnType.NUMERICAL,
              col_idx = jsnode['split'],)
      neg_child = _convert_xgboost_node_to_tfdf_node(
              jsnode['children'][0],
              )
      pos_child = _convert_xgboost_node_to_tfdf_node(
              jsnode['children'][1],
              )
      return tfdf.py_tree.node.NonLeafNode(
              condition = tfdf.py_tree.condition.NumericalHigherThanCondition(
                  feature = feature,
                  threshold = jsnode['split_condition'],
                  missing_evaluation = False,
                  ),
              pos_child = pos_child,
              neg_child = neg_child,
              )
  else:
      target_value = jsnode['leaf']
      scale_factor = 1.0
      if weight:
          scale_factor = weight
      node_value = tfdf.py_tree.value.RegressionValue(target_value*scale_factor)
      return tfdf.py_tree.node.LeafNode(value = node_value)

