#ifndef TUPLESET_SPECIFIER_HEADER
#define TUPLESET_SPECIFIER_HEADER


namespace pdb {

class TupleSetSpecifier {

public:
    // default constructor
    TupleSetSpecifier() {
        this->tupleSetName = "";
    }


    // constructor
    TupleSetSpecifier(std::string tupleSetName,
                      std::vector<std::string> columnNamesToKeep,
                      std::vector<std::string> columnNamesToApply) {

        this->tupleSetName = tupleSetName;
        this->columnNamesToKeep = columnNamesToKeep;
        this->columnNamesToApply = columnNamesToApply;
    }

    // return tuple set name
    std::string getTupleSetName() {
        return tupleSetName;
    }

    // return column names to keep in the output

    std::vector<std::string> getColumnNamesToKeep() {
        return columnNamesToKeep;
    }

    // return column names to apply a lambda

    std::vector<std::string> getColumnNamesToApply() {
        return columnNamesToApply;
    }

    // set tuple set name
    void setTupleSetName(std::string tupleSetName) {
        this->tupleSetName = tupleSetName;
    }

    // set column names to keep in the output
    void setColumnsToKeep(std::vector<std::string> columnsToKeep) {
        this->columnNamesToKeep = columnsToKeep;
    }

    // set column names to apply
    void setColumnsToApply(std::vector<std::string> columnsToApply) {
        this->columnsToApply = columnsToApply;
    }

private:
    // name of the the tuple set
    std::string tupleSetName;

    // column names in the tuple set to keep in the output
    std::vector<std::string> columnNamesToKeep;

    // column names in the tuple set to apply
    std::vector<std::string> columnNamesToApply;
};
}

#endif
