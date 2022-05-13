# do the inference via pySpark
from sklearn import *
import pandas as pd
import pickle
import joblib
import psycopg2
import numpy as np
import time
import pyspark
from pyspark.sql import SparkSession
from pyspark.sql.types import *
from pyspark.sql.functions import col, array_contains, udf
from pyspark.sql import SQLContext
from pyspark import SparkContext, SparkConf

filename = 'rf-10-8-6.pkl'

total_time_start = time.time()

#conf = SparkConf().set("spark.streaming.stopGracefullyOnShutdown","true") 
#sc = SparkContext(conf=conf)
sc = SparkContext()
sqlContext = SQLContext(sc)

#df = sqlContext.read.csv("/home/ubuntu/RF-benchmarks/higgs/pyspark/HIGGS_L.csv")


data2 = [(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0)]
for i in range(1,220000):
    data2.append((1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0))
#print(type(data2))
#print(type(data2[0]))

schema = StructType([ \
    StructField("_c1",FloatType(),True), \
    StructField("_c2",FloatType(),True), \
    StructField("_c3",FloatType(),True), \
    StructField("_c4", FloatType(), True), \
    StructField("_c5", FloatType(), True), \
    StructField("_c6", FloatType(), True), \
    StructField("_c7", FloatType(), True), \
    StructField("_c8", FloatType(), True), \
    StructField("_c9", FloatType(), True), \
    StructField("_c10", FloatType(), True), \
    StructField("_c11",FloatType(),True), \
    StructField("_c12",FloatType(),True), \
    StructField("_c13",FloatType(),True), \
    StructField("_c14", FloatType(), True), \
    StructField("_c15", FloatType(), True), \
    StructField("_c16", FloatType(), True), \
    StructField("_c17", FloatType(), True), \
    StructField("_c18", FloatType(), True), \
    StructField("_c19", FloatType(), True), \
    StructField("_c20", FloatType(), True), \
    StructField("_c21",FloatType(),True), \
    StructField("_c22",FloatType(),True), \
    StructField("_c23",FloatType(),True), \
    StructField("_c24", FloatType(), True), \
    StructField("_c25", FloatType(), True), \
    StructField("_c26", FloatType(), True), \
    StructField("_c27", FloatType(), True), \
    StructField("_c28", FloatType(), True) \
  ])

df = sqlContext.createDataFrame(data=data2,schema=schema)
df.printSchema()


serialized_model = open(filename,"rb")
model = pickle.load(serialized_model)
sc.broadcast(model)

def predictor(_c1,_c2,_c3,_c4,_c5,_c6,_c7,_c8,_c9,_c10,_c11,_c12,_c13,_c14,_c15,_c16,_c17,_c18,_c19,_c20,_c21,_c22,_c23,_c24,_c25,_c26,_c27,_c28):
    single_result = model.predict([[_c1,_c2,_c3,_c4,_c5,_c6,_c7,_c8,_c9,_c10,_c11,_c12,_c13,_c14,_c15,_c16,_c17,_c18,_c19,_c20,_c21,_c22,_c23,_c24,_c25,_c26,_c27,_c28]])
    #print(type(single_result))
    #print(type(single_result[0]))
    #print(single_result[0])
    return float(single_result[0])

udf_predictor = udf(predictor, FloatType())
df_predictor = df.withColumn("prediction",udf_predictor(df._c1,df._c2,df._c3,df._c4,df._c5,df._c6,df._c7,df._c8,df._c9,df._c10,df._c11,df._c12,df._c13,df._c14,df._c15,df._c16,df._c17,df._c18,df._c19,df._c20,df._c21,df._c22,df._c23,df._c24,df._c25,df._c26,df._c27,df._c28))

df_results = df_predictor.select('prediction')
df_results.write.csv('results')

#pandasDF = df_results.toPandas()
#pandasDF.to_csv('results.csv')

total_time_end=time.time()
print('total time cost',(total_time_end-total_time_start)*1000,'ms')