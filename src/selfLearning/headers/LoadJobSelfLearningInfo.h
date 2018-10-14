#ifndef LOAD_JOB_SELF_LEARNING_INFO_H
#define LOAD_JOB_SELF_LEARNING_INFO_H


namespace pdb {

//information to describe a load job that simply creates data
class LoadJobSelfLearningInfo {

public:

    LoadJobSelfLearningInfo () {
    }

    void setLoadJobName (std::string loadJobName) {
         this->loadJobName = loadJobName;
    }


    void setAvgSize (size_t avgSize) {
         this->avgSize = avgSize;
    }

    void setNumInstances (int numInstances) {
         this->numInstances = numInstances;
    }


    std::string getLoadJobName () {
         return loadJobName;
    }

    size_t getAvgSize () {
         return avgSize;
    }

    int getNumInstances () {
         return numInstances;
    }

    
   
     
private:

    std::string loadJobName;

    size_t avgSize;

    int numInstances;


};

}


#endif
