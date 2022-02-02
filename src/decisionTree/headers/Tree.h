#ifndef TREE_H
#define TREE_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "TreeNode.h"
#include "PDBClient.h"
#include "StorageClient.h"

// this file describes a decision tree

using namespace pdb;

namespace decisiontree{

	class Tree: public Object {

	public:

		ENABLE_DEEP_COPY

		pdb::Vector<pdb::Handle<decisiontree::Node>> tree;

		Tree() {}
		~Tree() {}

		Tree(pdb::Vector<pdb::Handle<decisiontree::Node>> treeIn){
			tree = treeIn;
		}

		pdb::Vector<pdb::Handle<decisiontree::Node>> get_tree() {
			return tree;
		}

		void set_tree(pdb::Vector<pdb::Handle<decisiontree::Node>> treeIn) {
			tree = treeIn;
		}
	};
}

#endif