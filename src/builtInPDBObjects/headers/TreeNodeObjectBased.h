
#ifndef NETSDB_TREE_NODE_OBJECT_BASED_H
#define NETSDB_TREE_NODE_OBJECT_BASED_H


#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <filesystem>
#include <future>
#include <thread>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <algorithm>
#include <map>
#include <set>
#include "TensorBlock2D.h"
#include "PDBClient.h"
#include "StorageClient.h"

// PRELOAD %TreeNodeObjectBased%

namespace pdb
{

    class TreeNodeObjectBased : public Object {
    
    public:

         TreeNodeObjectBased () {
	    leftChild = nullptr;
            rightChild = nullptr;
	 }

	 ~TreeNodeObjectBased () {
            leftChild = nullptr;
	    rightChild = nullptr;
         }

	 // normally these would be defined by the ENABLE_DEEP_COPY macro, but because
         // TreeNode is the one variable-sized type that we allow, we need to manually override
         // these methods
         void setUpAndCopyFrom(void* target, void* source) const override {
             new (target) TreeNodeObjectBased ();
             TreeNodeObjectBased& fromMe = *((TreeNodeObjectBased *) source);
             TreeNodeObjectBased& toMe = *((TreeNodeObjectBased *) target);
             if (fromMe.leftChild == nullptr)
                 toMe.leftChild = nullptr;
	     else {
	         toMe.leftChild = makeObject<TreeNodeObjectBased>();
		 toMe.leftChild = fromMe.leftChild;
	     }
	     if (fromMe.rightChild == nullptr)
                 toMe.rightChild = nullptr;
             else {
                 toMe.rightChild = makeObject<TreeNodeObjectBased>();
                 toMe.rightChild = fromMe.rightChild;
             }
             toMe.index = fromMe.index;
             toMe.returnClass = fromMe.returnClass;
         }

	 void deleteObject(void* deleteMe) override {
             deleter(deleteMe, this);
         }

         size_t getSize(void* forMe) override {
             return sizeof(TreeNodeObjectBased);
         }

         void print() {
	
	     std::cout << "index = " << index << ", returnClass = " << returnClass << std::endl;
	     if (leftChild != nullptr)
		 leftChild->print(); 
	     if (rightChild != nullptr)
		 rightChild->print();
	 }


         int index = -1;
         Handle<TreeNodeObjectBased> leftChild = nullptr;
         Handle<TreeNodeObjectBased> rightChild = nullptr;
	 // returnClass will be the vaule to compare while this is not a leaf node
         float returnClass =0.0; 
    };

}

#endif // NETSDB_TREENODEOBJECTBASED_H
