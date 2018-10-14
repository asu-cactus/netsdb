//#ifndef PDB_QUERYINTERMEDIARYREP_SETNAMEIR_H
//#define PDB_QUERYINTERMEDIARYREP_SETNAMEIR_H
//
//#include "Handle.h"
//#include "PDBString.h"
//#include "SetExpressionIr.h"
//
// using pdb::Handle;
// using pdb::String;
//
// namespace pdb_detail
//{
//    /**
//     * A PDB set identified by database name and set name.
//     */
//    class SetNameIr : public SetExpressionIr
//    {
//
//    public:
//
//        /**
//         * Creates a set name from the given database name and set name.
//         *
//         * @param databaseName the database name.
//         * @param setName the set name.
//         * @return the set name.
//         */
//        SetNameIr(Handle<String> databaseName, Handle<String> setName);
//
//        // contract from super
//        void execute(SetExpressionIrAlgo &algo) override;
//
//        /**
//         * @return the name of the database containing the set.
//         */
//        Handle<String> getDatabaseName();
//
//        /**
//         * @return the name of the set.
//         */
//        Handle<String> getName();
//
//
//    private:
//
//        /**
//         * The name of the database that contains the set.
//         */
//        Handle<String> const _databaseName;
//
//        /**
//         * The name of the set in the database.
//         */
//        Handle<String> const _setName;
//    };
//}
//
//#endif //PDB_QUERYINTERMEDIARYREP_SETNAMEIR_H
