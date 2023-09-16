
#ifndef TUPLE_SET_MACHINE_H
#define TUPLE_SET_MACHINE_H

namespace pdb {

class TupleSetSetupMachine {

    // there is one entry here for each item in attsToIncludeInOutput
    // that entry tells us where to find that attribute in the input
    std::vector<int> matches;

    TupleSpec& inputSchema;

public:
    TupleSetSetupMachine(TupleSpec& inputSchema) : inputSchema(inputSchema) {}

    TupleSetSetupMachine(TupleSpec& inputSchema, TupleSpec& attsToIncludeInOutput)
        : inputSchema(inputSchema) {
        matches = match(attsToIncludeInOutput);
    }

    // gets a vector that tells us where all of the attributes match
    std::vector<int> match(TupleSpec& attsToMatch) {

        // find the positions of all of the matches
        std::vector<int> matches;
        for (auto& s : attsToMatch.getAtts()) {
            int counter = 0;
            for (auto& t : inputSchema.getAtts()) {
                if (s == t) {
                    matches.push_back(counter);
                    counter = -1;
                    break;
                }
                counter++;
            }
            if (counter != -1) {
                std::cout << "This is bad... could not find a matching attribute\n";
                std::cout << "Atts to match was: " << attsToMatch << "\n";
                std::cout << "Input schema was: " << inputSchema << "\n";
            }
        }
        return matches;
    }

    // sets up the output tuple by copying over all of the atts that we need to, and setting the
    // output
    void setup(TupleSetPtr input, TupleSetPtr output) {

        // first, do a shallow copy of all of the atts that are being copied over
        int counter = 0;
        for (auto& i : matches) {

            // copy the column over, deleting the old one, if necessary
            output->copyColumn(input, i, counter++);
        }
    }

    // this is used by a join to replicate a bunch of input columns
    void replicate(TupleSetPtr input,
                   TupleSetPtr output,
                   std::vector<uint32_t>& counts,
                   int offset) {

        // first, do a shallow copy of all of the atts that are being copied over
        int counter = 0;
        for (auto& i : matches) {
            output->replicate(input, i, counter + offset, counts);
            counter++;
        }
    }
};

using TupleSetSetupMachinePtr = std::shared_ptr<TupleSetSetupMachine>;
}

#endif
