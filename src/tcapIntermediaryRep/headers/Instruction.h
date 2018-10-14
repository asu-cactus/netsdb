#ifndef PDB_TCAPINTERMEDIARYREP_INSTRUCTION_H
#define PDB_TCAPINTERMEDIARYREP_INSTRUCTION_H

#include <memory>
#include <functional>

using std::function;
using std::shared_ptr;

namespace pdb_detail {
/**
 * The different vairants of Instruction
 */
enum InstructionType { filter, load, apply_function, apply_method, hoist, greater_than, store };

/**
 * Base class for a TCAP instruction.
 */
class Instruction {
public:
    /**
     * The type of the instruction.
     */
    const InstructionType instructionType;

    /**
     * @param type the type of the instruction.
     */
    Instruction(InstructionType type);

    /***
     * A visitation hook for visitor pattern.
     *
     * @param forLoad the Load case
     * @param forApplyFunc he ApplyFunction case
     * @param forApplyMethod he ApplyMethod case
     * @param forFilter  he Filter case
     * @param forHoist he Hoist case
     * @param forGreaterThan  he GreaterThan case
     * @param forStore he Store case
     */
    virtual void match(function<void(class Load&)> forLoad,
                       function<void(class ApplyFunction&)> forApplyFunc,
                       function<void(class ApplyMethod&)> forApplyMethod,
                       function<void(class Filter&)> forFilter,
                       function<void(class Hoist&)> forHoist,
                       function<void(class GreaterThan&)> forGreaterThan,
                       function<void(class Store&)> forStore) = 0;
};

typedef shared_ptr<Instruction> InstructionPtr;
}

#endif  // PDB_TCAPINTERMEDIARYREP_INSTRUCTION_H
