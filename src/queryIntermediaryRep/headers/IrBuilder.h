#ifndef PDB_QUERYINTERMEDIARYREP_IRBUILDER_H
#define PDB_QUERYINTERMEDIARYREP_IRBUILDER_H

#include <list>
#include <memory>

#include "PDBVector.h"
#include "Handle.h"
#include "QueryBase.h"
#include "QueryGraphIr.h"

using std::list;
using std::shared_ptr;

using pdb::Handle;
using pdb::QueryBaseHdl;
using pdb::Vector;

namespace pdb_detail {
/**
 * Translates the given usery query graph into an equivalent logical query plan graph.
 *
 * Expects the given graph to have only a single sink identified by the querySink parameter.
 *
 * @param querySink The user query graph as identified by its single sink.
 * @return a corresponding logical query plan, or nullptr if there is a translation error
 */
QueryGraphIrPtr buildIrSingle(QueryBaseHdl querySink);

/**
 * Translates the given usery query graph into an equivalent logical query plan graph.
 *
 * Expects the given graph to have all sinks identified by the querySink parameter.
 *
 * @param querySinks The user query graph as identified by its sinks.
 * @return a corresponding logical query plan, or nullptr if there is a translation error
 */
QueryGraphIrPtr buildIr(list<QueryBaseHdl>& querySinks);

/**
 * Translates the given usery query graph into an equivalent logical query plan graph.
 *
 * Expects the given graph to have all sinks identified by the querySink parameter.
 *
 * @param querySinks The user query graph as identified by its sinks.
 * @return a corresponding logical query plan, or nullptr if there is a translation error
 */
QueryGraphIrPtr buildIr(Handle<Vector<QueryBaseHdl>> querySinks);
}

#endif  // PDB_QUERYINTERMEDIARYREP_IRBUILDER_H
