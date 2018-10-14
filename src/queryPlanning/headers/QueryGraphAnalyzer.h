#ifndef QUERY_GRAPH_ANALYZER_HEADER
#define QUERY_GRAPH_ANALYZER_HEADER

#include "Computation.h"
#include "Handle.h"
#include "InputTupleSetSpecifier.h"
#include "PDBVector.h"
#include <vector>

namespace pdb {


// This class encapsulates the analyzer to user query graph
// This class is also called a TCAP compiler, 
// and it translates a user query graph into a TCAP program.

// A TCAP program has following format:
// $TargetTupleSetName($tupleNamesToKeep, $newTupleName) <=
// $TCAPOperatorName($SourceTupleSetName($tupleNamesToApply),
//                   $SourceTupleSetName($tupleNamesToKeep),
//                   $ComputationName,
//                   $LambdaName)            

// Currently supported TCAP operators include:
// Apply, Aggregate, Join, ScanSet, WriteSet, HashLeft, HashRight
// HashOne, Flatten, Filter and so on.

// Note that the user query graph should not have loops

class QueryGraphAnalyzer {

public:
  // constructor
  QueryGraphAnalyzer(std::vector<Handle<Computation>> &queryGraph);

  // constructor
  QueryGraphAnalyzer(Handle<Vector<Handle<Computation>>> queryGraph);

  // to convert user query to a tcap string
  std::string parseTCAPString();

  // to traverse the sub-tree and put each traversed computation to a vector
  void parseComputations(std::vector<Handle<Computation>> &computations,
                         Handle<Computation> sink);

  // to convert user query to a pdb::Vector of computations
  // this method will invoke makeObject, but will not allocate allocation blocks
  // you must ensure current allocation block has sufficient memory before
  // invoking this method
  void parseComputations(std::vector<Handle<Computation>> &computations);

  // to traverse from a graph sink recursively
  void traverse(std::vector<std::string> &tcapStrings, Handle<Computation> sink,
                std::vector<InputTupleSetSpecifier> inputTupleSets,
                int &computationLabel, std::string &outputTupleSetName,
                std::vector<std::string> &outputColumnNames,
                std::string &addedOutputColumnName);

  // to clear traversal marks on the subtree rooted at sink
  void clearGraphMarks(Handle<Computation> sink);

  // to clear all traversal marks
  void clearGraphMarks();

private:
  // user query graph
  std::vector<Handle<Computation>> queryGraph;
};
}

#endif
