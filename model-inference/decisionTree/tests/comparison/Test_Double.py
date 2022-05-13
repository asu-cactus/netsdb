# run examples:
# python3 Test_Double.py C ../higgs/higgs_0.txt ../higgs/higgs_1.txt
# python3 Test_Double.py C ../higgs/higgs_0.txt ../higgs/higgs_1.txt ../higgs/higgs_2.txt ../higgs/higgs_3.txt ../higgs/higgs_4.txt ../higgs/higgs_5.txt ../higgs/higgs_6.txt ../higgs/higgs_7.txt ../higgs/higgs_8.txt ../higgs/higgs_9.txt
import pandas as pd
import numpy as np
import random
import sys
import time
import math
from joblib import Parallel, delayed

class Node:
    def _initialize_instance_fields(self):
        self.nodeID = 0
        self.indexID = 0
        self.isLeaf = False
        self.leftChild = 0
        self.rightChild = 0
        self.returnClass = np.float64(0.0)

    def __init__(self, nodeIDIn, indexIDIn, isLeafIn, leftChildIn, rightChildIn, returnClassIn):
        self._initialize_instance_fields()
        self.nodeID = nodeIDIn
        self.indexID = indexIDIn
        self.isLeaf = isLeafIn
        self.leftChild = leftChildIn
        self.rightChild = rightChildIn
        self.returnClass = returnClassIn

    def close(self):
        pass

    def get_node(self):
        return self.nodeID

    def get_index(self):
        return self.indexID

    def get_isLeaf(self):
        return self.isLeaf

    def get_leftChild(self):
        return self.leftChild

    def get_rightChild(self):
        return self.rightChild

    def get_returnClass(self):
        return self.returnClass

    def set_node(self, nodeIDIn):
        self.nodeID = nodeIDIn

    def set_index(self, indexIDIn):
        self.indexID = indexIDIn

    def set_isLeaf(self, isLeafIn):
        self.isLeaf = isLeafIn

    def set_leftChild(self, leftChildIn):
        self.leftChild = leftChildIn

    def set_rightChild(self, rightChildIn):
        self.rightChild = rightChildIn

    def set_returnClass(self, returnClassIn):
        self.returnClass = returnClassIn

    def equals_to(self, nodeID):
        return (self.nodeID == nodeID)

def most_common(input_array):
    most_value = input_array[0]
    max = 1
    for i in range(len(input_array)):
        t = 1
        for j in range(len(input_array)):
            if(input_array[i]==input_array[j]):
                t = t + 1
        if(t > max):
            max = t
            most_value = input_array[i]
    return most_value

def predict(tree, inputs):
    thisResultMatrix = np.ndarray(shape=(100,1), dtype='float64')
    for i in range(100):
        treeNode = tree[0]
        while treeNode.isLeaf == False:
            inputValue = inputs[i][treeNode.indexID]
            if inputValue <= treeNode.returnClass:
                treeNode = tree[(treeNode.leftChild)]
            else:
                treeNode = tree[(treeNode.rightChild)]
        thisResultMatrix[i][0] = treeNode.returnClass
    return thisResultMatrix


def main():
    args = sys.argv[1:]
    numTrees = len(args) - 1
    model_type = sys.argv[1]

    #print("number of trees: " + str(numTrees))
    forest = []

    n = 2
    while n <= numTrees + 1:
        inputFileName = sys.argv[n]

        relationships = []
        innerNodes = []
        leafNodes = []

        with open(inputFileName) as f:
            lines = f.readlines()
        
        leftLine = True
        rightLine = True
        
        for line in lines:
            if line.find("digraph Tree {") != -1:
                continue
            elif line.find("node [shape=box] ;") != -1:
                continue
            elif line.find("}") != -1:
                continue
            else:
                position = line.find("->")
                if position != -1:
                    if line.find("0 -> 1") != -1 and line.find("True") != -1:
                        rightLine = False
                    else:
                        leftLine = True
                    relationships.append(line)
                else:
                    if line.find("X") != -1:
                        innerNodes.append(line)
                    else:
                        leafNodes.append(line)
            #print("reading one line")
        f.close()
        #print("finish reading one file for one tree")
        n += 1
    #print("finish reading the files")
    
        tree = []
        i = 0
        while i < len(innerNodes):
            thisLine = innerNodes[i]
            nodeID = 0
            indexID = 0
            returnClass = np.float64(0.0)
            findEndPosition = thisLine.index("[label=")
            if findEndPosition != -1:
                nodeID = int(thisLine[0:findEndPosition-1])
            findStartPosition = thisLine.find("X")
            findEndPosition = thisLine.index("]")
            if findStartPosition != -1 and findEndPosition != -1:
                indexID = int(thisLine[findStartPosition+2:findEndPosition])
            findStartPosition = thisLine.find("<=")
            findEndPosition = thisLine.index("ngini")
            if findStartPosition != -1 and findEndPosition != -1:
                returnClass = np.float64(thisLine[findStartPosition+3:findEndPosition-1])
            tree.append(Node(nodeID, indexID, False, -1, -1, returnClass))
            i += 1

        i = 0
        while i < len(leafNodes):
            thisLine = leafNodes[i]
            nodeID = 0
            returnClass = np.float64(-1.0)
            findEndPosition = thisLine.index("[label=\"gini")
            if findEndPosition != -1:
                nodeID = int(thisLine[0:findEndPosition-1])
            if model_type == "C":
                findStartPosition = thisLine.find("nvalue = ")
                findMidPosition = thisLine.index(",")
                findEndPosition = thisLine.index("]\"] ;")
                if findStartPosition != -1 and findMidPosition != -1 and findEndPosition != -1:
                    firstValue = np.float64(thisLine[findStartPosition+10:findMidPosition])
                    secondValue = np.float64(thisLine[findMidPosition+2:findEndPosition])
                    if firstValue >= secondValue:
                        returnClass = np.float64(1.0)
                    else:
                        returnClass = np.float64(2.0)
            elif model_type == "R":
                findStartPosition = thisLine.find("nvalue = ")
                findEndPosition = thisLine.rindex("\"] ;")
                if findStartPosition!= -1 and findEndPosition != -1:
                    returnClass = float(thisLine[findStartPosition+9:findEndPosition])
            else:
                print("category error", end = '')
                print("\n", end = '')
                exit(1)
            tree.append(Node(nodeID, -1, True, -1, -1, returnClass))
            i += 1

        i = 0
        while i < len(relationships):
            nodeID = 0
            thisChild = 0
            thisLine = relationships[i]
            findMidPosition = thisLine.index("->")
            if findMidPosition != -1:
                nodeID = int(thisLine[0:findMidPosition-1])
            if nodeID == 0:
                findEndPosition = thisLine.index(" [")
                if findEndPosition != -1:
                    thisChild = int(thisLine[findMidPosition+3:findEndPosition])
            else:
                findEndPosition = thisLine.index(" ;")
                if findEndPosition != -1:
                    thisChild = int(thisLine[findMidPosition+3:findEndPosition])
        
            for j in range(len(tree)):
                if tree[j].nodeID == nodeID:
                    position = j
                else:
                    position = -1
            if position < len(tree) - 1:
                if tree[j].get_leftChild() == -1:
                    tree[j].set_leftChild(thisChild)
                else:
                    tree[j].set_rightChild(thisChild)
            else:
                print("construct error", end = '')
                print("\n", end = '')
                exit(1)
            i += 1
        forest.append(tree)

    print("numbers of trees in the forest: ", end = '')
    print(len(forest), end = '')
    print("\n", end = '')
    print("numbers of nodes in each tree: ", end = '')
    print("\n", end = '')
    i = 0
    while i < len(forest):
        print("numbers of nodes in tree ", end = '')
        print(i, end = '')
        print(" is ", end = '')
        print(len(forest[i]), end = '')
        print("\n", end = '')
        i += 1
    
    # set the input array size
    inNumRow = 100
    inNumFeature = 28

    # randomly generate the inputs
    inputs = np.ndarray(shape=(inNumRow,inNumFeature), dtype='float64', order='F')

    for i in range(inNumRow):
        for j in range(inNumFeature):
            inputs[i][j] = np.float64(round(random.uniform(0.0, 1.0), 3))
    
    # set the final output array
    resultMatrix = np.ndarray(shape=(inNumRow,1), dtype='float64', order='F')

    # do the inference
    time_start = time.time()
    results = Parallel(n_jobs=2)(delayed(predict)(tree, inputs) for tree in forest)
    total_results = np.array(results)
    for i in range(100):
        one_result = np.ravel(total_results[:,i])
        resultMatrix[i] = np.unique(one_result).argmax()
    time_end=time.time()
    print("Double in Python Inference Time: ", end = '')
    print((time_end-time_start), " secs.")
    print("\n", end = '')

if __name__ == '__main__':
    sys.exit(main())