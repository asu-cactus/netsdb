
#ifndef COMP_CLASSES_H
#define COMP_CLASSES_H

#include "TupleSpec.h"
#include "AtomicComputationList.h"

// NOTE: these are NOT part of the pdb namespace because they need to be included in an "extern
// C"...
// I am not sure whether that is possible... perhaps we try moving them to the pdb namespace later.

// this is a computation that applies a lambda to a tuple set
struct ApplyLambda : public AtomicComputation {

private:
    std::string lambdaName;

public:
    ~ApplyLambda() {}

    ApplyLambda(TupleSpec& input,
                TupleSpec& output,
                TupleSpec& projection,
                std::string nodeName,
                std::string lambdaNameIn)
        : AtomicComputation(input, output, projection, nodeName), lambdaName(lambdaNameIn) {}

    std::string getAtomicComputationType() override {
        return std::string("Apply");
    }

    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // the output from the apply is:
        //
        // (set of projection atts) (new attribute created from apply)
        //
        // find where the attribute appears in the outputs
        int counter = findPosInOutputAtts(attName);

        // if the attribute we are asking for is at the end (where the result of the lambda
        // application goes)
        // then we asked for it
        if (counter == getOutput().getAtts().size() - 1) {
            return std::make_pair(getComputationName(), lambdaName);
        }

        // otherwise, find our parent
        return allComps.getProducingAtomicComputation(getProjection().getSetName())
            ->findSource((getProjection().getAtts())[counter], allComps);
    }

    // returns the name of the lambda we are supposed to apply
    std::string& getLambdaToApply() {
        return lambdaName;
    }

    friend std::ostream& operator<<(std::ostream& os, const AtomicComputationList& printMe);
};

// this is a computation that applies a hash to a particular attribute in a tuple set
struct HashLeft : public AtomicComputation {

private:
    std::string lambdaName;

public:
    ~HashLeft() {}

    HashLeft(TupleSpec& input,
             TupleSpec& output,
             TupleSpec& projection,
             std::string nodeName,
             std::string lambdaNameIn)
        : AtomicComputation(input, output, projection, nodeName), lambdaName(lambdaNameIn) {}

    std::string getAtomicComputationType() override {
        return std::string("HashLeft");
    }

    // returns the name of the lambda we are supposed to apply
    std::string& getLambdaToApply() {
        return lambdaName;
    }

    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // The output from the hash should be
        //
        // (projection atts) (hash value)
        //

        // find where the attribute appears in the outputs
        int counter = findPosInOutputAtts(attName);

        // if the attribute we are asking for is at the end (where the result of the lambda
        // application goes)
        // then we asked for it
        if (counter == getOutput().getAtts().size() - 1) {
            return allComps.getProducingAtomicComputation(getInput().getSetName())
            ->findSource((getInput().getAtts())[0], allComps);
        }

        // otherwise, find our parent
        return allComps.getProducingAtomicComputation(getProjection().getSetName())
            ->findSource((getProjection().getAtts())[counter], allComps);
    }

    friend std::ostream& operator<<(std::ostream& os, const AtomicComputationList& printMe);
};

// this is a computation that applies a lambda to a tuple set
struct HashRight : public AtomicComputation {

private:
    std::string lambdaName;

public:
    ~HashRight() {}

    HashRight(TupleSpec& input,
              TupleSpec& output,
              TupleSpec& projection,
              std::string nodeName,
              std::string lambdaNameIn)
        : AtomicComputation(input, output, projection, nodeName), lambdaName(lambdaNameIn) {}

    std::string getAtomicComputationType() override {
        return std::string("HashRight");
    }

    // returns the name of the lambda we are supposed to apply
    std::string& getLambdaToApply() {
        return lambdaName;
    }

    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // The output from the hash should be
        //
        // (projection atts) (hash value)
        //

        // find where the attribute appears in the outputs
        int counter = findPosInOutputAtts(attName);

        // if the attribute we are asking for is at the end (where the result of the lambda
        // application goes)
        // then we asked for it
        if (counter == getOutput().getAtts().size() - 1) {
            return allComps.getProducingAtomicComputation(getInput().getSetName())
            ->findSource((getInput().getAtts())[0], allComps);
        }

        // otherwise, find our parent
        return allComps.getProducingAtomicComputation(getProjection().getSetName())
            ->findSource((getProjection().getAtts())[counter], allComps);
    }

    friend std::ostream& operator<<(std::ostream& os, const AtomicComputationList& printMe);
};


// this is a computation that adds 1  to each tuple of a tuple set
struct HashOne : public AtomicComputation {


public:
    ~HashOne() {}

    HashOne(TupleSpec& input, TupleSpec& output, TupleSpec& projection, std::string nodeName)
        : AtomicComputation(input, output, projection, nodeName) {

    }

    std::string getAtomicComputationType() override {
        return std::string("HashOne");
    }


    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // The output from the hash should be
        //
        // (projection atts) (hash value)
        //

        // find where the attribute appears in the outputs
        // otherwise, find our parent
        int counter = findPosInOutputAtts(attName);
        if (counter >= getProjection().getAtts().size()) {
           return allComps.getProducingAtomicComputation(getProjection().getSetName())
            ->findSource((getProjection().getAtts())[0], allComps);
        } else {
           return allComps.getProducingAtomicComputation(getProjection().getSetName())
            ->findSource((getProjection().getAtts())[counter], allComps);

        }
    }

    friend std::ostream& operator<<(std::ostream& os, const AtomicComputationList& printMe);
};


// this is a computation that flatten each tuple of a tuple set
struct Flatten : public AtomicComputation {


public:
    ~Flatten() {}

    Flatten(TupleSpec& input, TupleSpec& output, TupleSpec& projection, std::string nodeName)
        : AtomicComputation(input, output, projection, nodeName) {

    }

    std::string getAtomicComputationType() override {
        return std::string("Flatten");
    }


    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // The output from the hash should be
        //
        // (projection atts) (hash value)
        //
        // find where the attribute appears in the outputs
        int counter = findPosInOutputAtts(attName);

        if (counter == getOutput().getAtts().size() - 1) {
            return std::make_pair(getComputationName(), std::string(""));
        }

        // otherwise, find our parent

        return allComps.getProducingAtomicComputation(getProjection().getSetName())
            ->findSource((getProjection().getAtts())[counter], allComps);
    }

    friend std::ostream& operator<<(std::ostream& os, const AtomicComputationList& printMe);
};


// this is a computation that performs a filer over a tuple set
struct ApplyFilter : public AtomicComputation {

public:
    ~ApplyFilter() {}

    ApplyFilter(TupleSpec& input, TupleSpec& output, TupleSpec& projection, std::string nodeName)
        : AtomicComputation(input, output, projection, nodeName) {
    }

    std::string getAtomicComputationType() override {
        return std::string("Filter");
    }

    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // the output from the filter should be identical to the set of projection attributes
        // find where the attribute appears in the outputs
        int counter = findPosInOutputAtts(attName);

        // otherwise, find our parent
        return allComps.getProducingAtomicComputation(getProjection().getSetName())
            ->findSource((getProjection().getAtts())[counter], allComps);
    }
};

// this is a computation that aggregates a tuple set
struct ApplyAgg : public AtomicComputation {

public:
    ~ApplyAgg() {}

    ApplyAgg(TupleSpec& input, TupleSpec& output, TupleSpec& projection, std::string nodeName)
        : AtomicComputation(input, output, projection, nodeName) {}

    std::string getAtomicComputationType() override {
        return std::string("Aggregate");
    }

    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // The output from the aggregate should be a single attribute
        // find where the attribute appears in the outputs
        int counter = findPosInOutputAtts(attName);

        // if the attribute we are asking for is at the end, it means it's produced by this
        // aggregate
        // then we asked for it
        if (counter == 0) {
            return std::make_pair(getComputationName(), std::string(""));
        }

        // if it is not at the end, if makes no sense
        std::cout << "How did we ever get here trying to find an attribute produced by an agg??\n";
        exit(1);
    }
};

// this is a computation that produces a tuple set by scanning a set stored in the database
struct ScanSet : public AtomicComputation {

    std::string dbName;
    std::string setName;

public:
    ~ScanSet() {}

    ScanSet(TupleSpec& output, std::string dbName, std::string setName, std::string nodeName)
        : AtomicComputation(TupleSpec(), output, TupleSpec(), nodeName),
          dbName(dbName),
          setName(setName) {}

    std::string getAtomicComputationType() override {
        return std::string("Scan");
    }

    std::string& getDBName() {
        return dbName;
    }

    std::string& getSetName() {
        return setName;
    }

    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // The output from the scan should be a single attribute
        // find where the attribute appears in the outputs
        int counter = findPosInOutputAtts(attName);

        // if the attribute we are asking for is at the end (where the result of the lambda
        // application goes)
        // then we asked for it
        if (counter == 0) {
            return std::make_pair(getComputationName(), std::string(""));
        }

        // if it is not at the end, if makes no sense
        std::cout
            << "How did we ever get here trying to find an attribute produced by a scan set??\n";
        exit(1);
    }
};

// this is a computation that writes out a tuple set
struct WriteSet : public AtomicComputation {

    std::string dbName;
    std::string setName;

public:
    ~WriteSet() {}

    WriteSet(TupleSpec& input,
             TupleSpec& output,
             TupleSpec& projection,
             std::string dbName,
             std::string setName,
             std::string nodeName)
        : AtomicComputation(input, output, projection, nodeName),
          dbName(dbName),
          setName(setName) {}

    std::string getAtomicComputationType() override {
        return std::string("WriteSet");
    }

    std::string& getDBName() {
        return dbName;
    }

    std::string& getSetName() {
        return setName;
    }

    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {
        std::cout << "How did we ever get to a write set trying to find an attribute??\n";
        exit(1);
    }
};

struct ApplyJoin : public AtomicComputation {

    TupleSpec rightInput;
    TupleSpec rightProjection;
    // JiaNote: added below for physical planning
    // if traversed is set to true, we know that one input has been processed, and the other input
    // can go through the pipeline
    bool traversed = false;
    // JiaNote: added below for hash partitioned join
    bool toPartitionLHS = false;

public:
    ApplyJoin(TupleSpec& output,
              TupleSpec& lInput,
              TupleSpec& rInput,
              TupleSpec& lProjection,
              TupleSpec& rProjection,
              std::string nodeName)
        : AtomicComputation(lInput, output, lProjection, nodeName),
          rightInput(rInput),
          rightProjection(rProjection) {
        traversed = false;
        toPartitionLHS = false;
    }

    TupleSpec& getRightProjection() {
        return rightProjection;
    }

    TupleSpec& getRightInput() {
        return rightInput;
    }

    std::string getAtomicComputationType() override {
        return std::string("JoinSets");
    }

    bool isTraversed() {
        return this->traversed;
    }

    void setTraversed(bool traversed) {
        this->traversed = traversed;
    }

    bool isPartitioningLHS() {
        return this->toPartitionLHS;
    }

    void setPartitioningLHS(bool toPartitionLHS) {
        this->toPartitionLHS = toPartitionLHS;
    }


    std::pair<std::string, std::string> findSource(std::string attName,
                                                   AtomicComputationList& allComps) override {

        // The output from the join should be
        //
        // (left projection atts) (right projection atts)
        //
        // so find where the attribute in question came from
        int counter = findPosInOutputAtts(attName);

        // if it came from the left, then we recurse and find it
        if (counter < getProjection().getAtts().size()) {
            return allComps.getProducingAtomicComputation(getProjection().getSetName())
                ->findSource((getProjection().getAtts())[counter], allComps);

            // otherwise, if it came from the right, recurse and find it
        } else if (counter < getProjection().getAtts().size() + rightProjection.getAtts().size()) {
            return allComps.getProducingAtomicComputation(rightProjection.getSetName())
                ->findSource(
                    (rightProjection.getAtts())[counter - getProjection().getAtts().size()],
                    allComps);

        } else {
            std::cout << "Why in the heck did we not find the producer when checking a join!!??\n";
            exit(1);
        }
    }
};


// this is a computation that partitions a tuple set
struct ApplyPartition : public AtomicComputation {

 public:
  ~ApplyPartition() {}

  ApplyPartition(TupleSpec &input, TupleSpec &output, TupleSpec &projection, std::string nodeName)
      : AtomicComputation(input, output, projection, nodeName) {}

  std::string getAtomicComputationType() override {
    return std::string("Partition");
  }

  std::pair<std::string, std::string> findSource(std::string attName,
                                                 AtomicComputationList &allComps) override {

    // The output from the partition should be a single attribute
    // find where the attribute appears in the outputs
    int counter = findPosInOutputAtts(attName);

    // if the attribute we are asking for is at the end, it means it's produced by this
    // aggregate
    // then we asked for it
    if (counter == 0) {
      return std::make_pair(getComputationName(), std::string(""));
    }

    // if it is not at the end, if makes no sense
    std::cout << "How did we ever get here trying to find an attribute produced by an agg??\n";
    exit(1);
  }
};

#endif
