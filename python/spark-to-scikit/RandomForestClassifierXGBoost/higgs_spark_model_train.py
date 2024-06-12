from pyspark import SparkContext
from pyspark.sql import SQLContext
from pyspark.sql.types import StructType, StructField, DoubleType
from pyspark.ml.classification import GBTClassifier
from pyspark.ml.feature import VectorAssembler

def save_model(model_string, filename):
    with open(filename, "w") as text_file:
        text_file.write(model_string)

if __name__ == '__main__':
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

    df = sqlContext.read.format('com.databricks.spark.csv').schema(schema).load("HIGGS_100.csv/part-00000-758749fa-cbbd-473e-b1cd-4d06fd714edd-c000.csv").limit(100)
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
    
    rf = GBTClassifier(featuresCol=features_column, labelCol=label_column, maxIter=2)
    train, test = df.randomSplit([0.7, 0.3], seed = 2018)
    rf_model = rf.fit(train)

    # export model as string
    model_string = rf_model.toDebugString
    rf_model.setThresholds([0.5, 0.5])
    thresholds = rf_model.getThresholds()
    thresholds_string = " ".join(str(x) for x in thresholds)
    save_model(thresholds_string + '\n' + model_string, 'higgs_spark_model.txt')

    # save model using spark
    rf_model.write().overwrite().save('higgs_spark_model')

    # print predictions
    predictions = rf_model.transform(test)
    predictions.select(label_column, "prediction", 'probability').show()

    # # save test rows as csv for validation
    # test = test.drop(features_column)
    # test.coalesce(1).write.mode("overwrite").option("header","true").csv("testdata_higgs_spark_model")