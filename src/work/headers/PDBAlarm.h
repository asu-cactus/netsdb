/*
 * File:   PDBAlarm.h
 * Author: Chris
 *
 * Created on September 25, 2015, 5:18 PM
 */

#ifndef PDBALARM_H
#define PDBALARM_H

enum PDBAlarm {
    WorkAllDone,
    GenericError,
    FatalServerError,
    ServerError,
    OpenFileError,
    BuildFeatureError,
    QueryError,
    HeartBeatError

};

#endif /* PDBALARM_H */
