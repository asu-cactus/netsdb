
#ifndef PDBOBJECT_PROTOTYPE_H
#define PDBOBJECT_PROTOTYPE_H

// What's the idea here?
//
// Well, we need to get each pdb template type into the v-table map.  Each template type will then
// be assigned a type ID that every instance of the template will share, no matter what the type
// parameterization.  The PDBTemplateBase within the template will be used to remember the exact
// type of the object inside of the template, so that we can perform operations on it at a later
// time.

//  PRELOAD %Object%

#endif
