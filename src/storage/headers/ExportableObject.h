#ifndef EXPORTABLE_OBJECT_H
#define EXPORTABLE_OBJECT_H

#include "Object.h"
#include <string>

class ExportableObject : public pdb::Object {

public:
    // to simply print to std::cout
    virtual void print() = 0;

    // to return the schema string of this object: e.g. the csv header line for csv format
    // format can be "csv", "json", "parquet" and so on
    virtual std::string toSchemaString(std::string format) = 0;

    // to return the value string of this object: e.g. a line of comma separated values for csv
    // format
    virtual std::string toValueString(std::string format) = 0;

    // one object may support multiple exporting formats
    // to return all supported formats
    virtual std::vector<std::string> getSupportedFormats() = 0;
};


#endif
