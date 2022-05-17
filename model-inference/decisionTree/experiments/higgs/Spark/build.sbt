name := "SparkTwoExperiments"

version := "1.0"

scalaVersion := "2.11.8"

val pmml4sVersion = "0.9.15"
val sparkVersion = "2.2.0"

libraryDependencies ++= Seq(
  "org.apache.spark" %% "spark-core" % sparkVersion,
  "org.apache.spark" %% "spark-sql" % sparkVersion,
  "org.apache.spark" %% "spark-mllib" % sparkVersion,
  "org.apache.spark" %% "spark-streaming" % sparkVersion,
  "org.apache.spark" %% "spark-hive" % sparkVersion,
  "org.jpmml" % "pmml-model" % "1.3.8",
  "org.jpmml" % "pmml-schema" % "1.3.8",
  "org.jpmml" % "jpmml-sparkml" % "1.2.7",
  "org.jpmml" % "jpmml-converter" % "1.2.6",
  "org.pmml4s" %%  "pmml4s" % pmml4sVersion,
  "org.apache.spark" %% "spark-core" % "1.4.0" % "provided"
)

assemblyMergeStrategy in assembly := {
 case PathList("META-INF", xs @ _*) => MergeStrategy.discard
 case x => MergeStrategy.first
}