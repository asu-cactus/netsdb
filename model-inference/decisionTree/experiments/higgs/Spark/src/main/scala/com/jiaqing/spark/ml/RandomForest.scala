import org.apache.spark.sql.SparkSession
import org.apache.spark.ml._
import org.apache.spark.ml.feature._
import org.apache.spark.ml.classification.RandomForestClassifier
import org.apache.spark.sql.functions._
import org.apache.spark.sql.SaveMode
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.types.IntegerType
import org.apache.spark.ml.tuning.ParamGridBuilder
import org.apache.spark.ml.param.ParamMap
import org.apache.spark.ml.tuning.CrossValidator
import org.apache.spark.ml.evaluation.BinaryClassificationEvaluator
import org.apache.spark.mllib.evaluation.MulticlassMetrics
import org.pmml4s.model.Model
import java.io._
import scala.collection.mutable.ListBuffer
import java.io.PrintWriter
import java.io.File
import scala.collection.JavaConversions._
import org.apache.spark.sql.types._
import org.apache.spark.sql.Row

object RandomForest {

  def main(args: Array[String]) {

    val sparkSession = SparkSession.builder.
      master("local[8]")
      .appName("example")
      .getOrCreate()

    sparkSession.sparkContext.setLogLevel("ERROR")

    var start_time = System.nanoTime()
/*
    val data2 = Seq(Row(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0))

    val schema = StructType(Array(
    StructField("_c1",FloatType,true),
    StructField("_c2",FloatType,true),
    StructField("_c3",FloatType,true),
    StructField("_c4", FloatType,true),
    StructField("_c5", FloatType,true),
    StructField("_c6", FloatType,true),
    StructField("_c7", FloatType,true),
    StructField("_c8", FloatType,true),
    StructField("_c9", FloatType,true),
    StructField("_c10", FloatType,true),
    StructField("_c11",FloatType,true),
    StructField("_c12",FloatType,true),
    StructField("_c13",FloatType,true),
    StructField("_c14", FloatType,true),
    StructField("_c15", FloatType,true),
    StructField("_c16", FloatType,true),
    StructField("_c17", FloatType,true),
    StructField("_c18", FloatType,true),
    StructField("_c19", FloatType,true),
    StructField("_c20", FloatType,true),
    StructField("_c21",FloatType,true),
    StructField("_c22",FloatType,true),
    StructField("_c23",FloatType,true),
    StructField("_c24", FloatType,true),
    StructField("_c25", FloatType,true),
    StructField("_c26", FloatType,true),
    StructField("_c27", FloatType,true),
    StructField("_c28", FloatType,true)
))

   val df = sparkSession.createDataFrame(data2,schema)
   df.printSchema()

*/
  //val df = sparkSession.read.option("header", "false").option("inferSchema", "true").csv("src/main/resources/HIGGS.csv")
   val randomForestModel = Model.fromFile("rf-10-8-6.pmml")
   val oneData = Array(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,25.0,26.0,27.0,28.0)
   val pw = new PrintWriter(new File("result.txt" ))
   for (w <- 0 to 22000000){
      val predictions = randomForestModel.predict(oneData)
      //val predictions = randomForestModel.predict(df[w])
      val resultsList = predictions.toList
      for (line <- resultsList){
        pw.write(String.valueOf(line))
      }
    }
    pw.close

    var end_time = System.nanoTime()

    println("total time: " + (end_time - start_time)/1000000 + " ms")
  }
}