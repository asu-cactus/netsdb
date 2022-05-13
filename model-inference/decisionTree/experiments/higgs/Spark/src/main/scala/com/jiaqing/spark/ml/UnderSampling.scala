package com.madhukaraphatak.spark.ml

import org.apache.spark.sql.SparkSession
import org.apache.spark.ml._
import org.apache.spark.ml.feature._
import org.apache.spark.ml.classification.LogisticRegression
import org.apache.spark.sql.functions._
import org.apache.spark.sql.SaveMode
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.types.IntegerType
import org.apache.spark.ml.tuning.ParamGridBuilder
import org.apache.spark.ml.param.ParamMap
import org.apache.spark.ml.tuning.CrossValidator
import org.apache.spark.ml.evaluation.BinaryClassificationEvaluator
import org.apache.spark.mllib.evaluation.MulticlassMetrics
import MLUtils._

/**
 * Undersampling for Credict Card Fraud
 *
 */
object UnderSampling {

  def main(args: Array[String]) {

    val sparkSession = SparkSession.builder.
      master("local[4]")
      .appName("example")
      .getOrCreate()

    sparkSession.sparkContext.setLogLevel("ERROR")
    //load train df
    // Download the data from : https://www.kaggle.com/dalpozz/creditcardfraud/downloads/creditcard.csv
    val df = sparkSession.read.option("header", "true").option("inferSchema", "true").csv("src/main/resources/creditcard.csv")
    df.printSchema()

    val amountVectorAssembler = new VectorAssembler().setInputCols(Array("Amount")).setOutputCol("Amount_vector")
    val standarScaler = new StandardScaler().setInputCol("Amount_vector").setOutputCol("Amount_scaled")
    val dropColumns = Array("Time","Amount","Class")
    
    val cols = df.columns.filter( column => !dropColumns.contains(column)) ++ Array("Amount_scaled")
    val vectorAssembler = new VectorAssembler().setInputCols(cols).setOutputCol("features")

    // pipeline 
    val logisticRegression = new LogisticRegression().setLabelCol("Class")
    val trainPipeline = new Pipeline().setStages(Array(amountVectorAssembler,standarScaler,vectorAssembler,logisticRegression))

    println("for imbalanced data")
    runPipeline(trainPipeline, df)

    val underSampledDf = underSampleDf(df)
    println("for balanced data")
    val balancedModel = runPipeline(trainPipeline, underSampledDf)
      
    println("balanced model for full data")
    printScores(balancedModel, df)

   }

  def underSampleDf(df:DataFrame) = {
    val fraudDf = df.filter("Class=1.0")
    val nonFraudDf = df.filter("Class=0.0")
    //random sample the nonFraud to match the value of fraud
    val sampleRatio = fraudDf.count().toDouble / df.count().toDouble
    val nonFraudSampleDf = nonFraudDf.sample(false, sampleRatio)
    fraudDf.unionAll(nonFraudSampleDf)
  }

  def runPipeline(pipeline:Pipeline, df:DataFrame):PipelineModel = {
    val (trainDf,crossDf) = trainTestSplit(df)
    val model = pipeline.fit(trainDf)
    printScores(model, crossDf)
    model
  }

  def printScores(model:PipelineModel, df:DataFrame) = {
    println("test accuracy with pipeline " + accuracyScore(model.transform(df), "Class", "prediction"))
    println("test recall for 1.0 is " + recall(model.transform(df), "Class", "prediction", 1.0))
  }
}
 
