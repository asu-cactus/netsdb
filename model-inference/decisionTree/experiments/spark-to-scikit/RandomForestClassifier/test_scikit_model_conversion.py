from sklearn.tree import export_text
import sklearn_json as skljson
import pandas as pd
from pyspark.ml.classification import RandomForestClassificationModel
from pyspark import SparkContext
from pyspark.sql import SQLContext
from pyspark.sql.types import StructType, StructField, DoubleType
from pyspark.ml.feature import VectorAssembler
from sklearn import tree
from matplotlib import pyplot as plt
import argparse

def spark_predictions(test_data_file_path):
    sc = SparkContext()
    sqlContext = SQLContext(sc)
    schema = StructType([ \
        StructField("label", DoubleType(),True), \
        StructField("f0", DoubleType(),True), \
        StructField("f1", DoubleType(),True), \
        StructField("f2", DoubleType(),True), \
        StructField("f3", DoubleType(), True), \
        StructField("f4", DoubleType(), True), \
        StructField("f5", DoubleType(), True), \
        StructField("f6", DoubleType(), True), \
        StructField("f7", DoubleType(), True), \
        StructField("f8", DoubleType(), True), \
        StructField("f9", DoubleType(), True), \
        StructField("f10", DoubleType(), True), \
        StructField("f11", DoubleType(),True), \
        StructField("f12", DoubleType(),True), \
        StructField("f13", DoubleType(),True), \
        StructField("f14", DoubleType(), True), \
        StructField("f15", DoubleType(), True), \
        StructField("f16", DoubleType(), True), \
        StructField("f17", DoubleType(), True), \
        StructField("f18", DoubleType(), True), \
        StructField("f19", DoubleType(), True), \
        StructField("f20", DoubleType(), True), \
        StructField("f21", DoubleType(),True), \
        StructField("f22", DoubleType(),True), \
        StructField("f23", DoubleType(),True), \
        StructField("f24", DoubleType(), True), \
        StructField("f25", DoubleType(), True), \
        StructField("f26", DoubleType(), True), \
        StructField("f27", DoubleType(), True), \
    ])

    df = sqlContext.read.format('com.databricks.spark.csv').option("header",True).schema(schema).load(test_data_file_path)
    features_column = 'features'
    label_column = 'label'
    feature_cols = ['f0', 'f1', 'f2', 'f3','f4','f5',
                'f6','f7','f8','f9','f10', 
                'f11','f12','f13','f14','f15',
                'f16','f17','f18','f19','f20',
                'f21', 'f22', 'f23', 'f24', 'f25', 
                'f26', 'f27']
    assembler = VectorAssembler(inputCols=feature_cols, outputCol=features_column)
    df = assembler.transform(df)
    spark_model = RandomForestClassificationModel.load('higgs_spark_model')
    predictions = spark_model.transform(df)
    return [int(row['prediction']) for row in predictions.select("prediction").collect()]

def json_to_scikit(json_filename):
    return skljson.from_json(json_filename)

def print_decision_path(X_test, clf):
    # First decsion tree in randam forest
    feature = clf.estimators_[0].tree_.feature
    threshold = clf.estimators_[0].tree_.threshold

    node_indicator = clf.estimators_[0].decision_path(X_test)
    leaf_id = clf.estimators_[0].apply(X_test)

    sample_id = 0
    # obtain ids of the nodes `sample_id` goes through, i.e., row `sample_id`
    node_index = node_indicator.indices[
        node_indicator.indptr[sample_id] : node_indicator.indptr[sample_id + 1]
    ]

    print("Rules used to predict sample {id}:\n".format(id=sample_id))
    for node_id in node_index:
        # continue to the next node if it is a leaf node
        if leaf_id[sample_id] == node_id:
            continue

        # check if value of the split feature for sample 0 is below threshold
        if X_test.iloc[sample_id, feature[node_id]] <= threshold[node_id]:
            threshold_sign = "<="
        else:
            threshold_sign = ">"

        print(
            "decision node {node} : (X_test[{sample}, {feature}] = {value}) "
            "{inequality} {threshold})".format(
                node=node_id,
                sample=sample_id,
                feature=feature[node_id],
                value=X_test.iloc[sample_id, feature[node_id]],
                inequality=threshold_sign,
                threshold=threshold[node_id],
            )
        )

def write_to_file(model_string, filename):
    with open(filename, "w") as text_file:
        text_file.write(model_string)

def get_scikit_mode_text(scikit_model):    
    scikit_model_text = ''
    for estimator in scikit_model.estimators_:
        scikit_model_text += export_text(estimator)
        scikit_model_text += "\n"
    return scikit_model_text

if __name__ == '__main__':
    # Test data file path
    parser = argparse.ArgumentParser(description='Parse test data file path')
    parser.add_argument('--filepath', dest='filepath', help='Path to testdata csv file.', required=True)
    args = parser.parse_args()

    test_data_file_path = args.filepath

    # Use scikit model for prediction in experiments
    scikit_model = json_to_scikit('converted_scikit_model.json')

    # Meta Data
    scikit_model_text = get_scikit_mode_text(scikit_model)
    write_to_file(scikit_model_text, 'metadata/higgs_scikit_model.txt')
    for idx, estimator in enumerate(scikit_model.estimators_):
        tree.plot_tree(estimator)
        plt.savefig('metadata/scikit_tree_image_' + str(idx) + '.png', dpi=300)

    # Test data
    df = pd.read_csv(test_data_file_path)
    df = df.drop('label', axis=1) # remove non feature first column 

    # Validate if two predictions 
    scikit_prediction_list = scikit_model.predict(df)
    spark_predictions_list = spark_predictions(test_data_file_path)
    print (scikit_prediction_list)
    print (spark_predictions_list)

    if (scikit_prediction_list == spark_predictions_list).all():
        print ("Hooray, Test case successful!")
    else:
        print ("Validation failed")
