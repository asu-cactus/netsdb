#ifndef DATA_CONTEXT_H
#define DATA_CONTEXT_H


//Information to describe a Set


namespace pdb {

class DataContext  {

public:

    //constructor
    DataContext(int producingStageId, long dataId, int indexInInputs) {

        this->producingStageId = producingStageId;
        this->dataId = dataId;
        this->indexInInputs = indexInInputs;

    }

    int producingStageId;
    long dataId;
    int indexInInputs; 

};

}
#endif
