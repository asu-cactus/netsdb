#!/bin/bash
# by Jia


PEM_FILE=$1



USERNAME=ubuntu


#remember to set environment variable: PDB_HOME first
if [ -z ${PDB_HOME} ]; 
    then echo "[ERROR] Please set PDB_HOME first";
else 
    echo "Your current PDB_HOME is: '$PDB_HOME'";
fi

#remember to provide your pem file as parameter
if [ -z ${PEM_FILE} ];
    then PEM_FILE=$PDB_HOME/conf/pdb.key;
    chmod -R 600 $PDB_HOME/conf/pdb.key
else
    echo "Your pem file is: '$PEM_FILE'";
    chmod -R 600 $PEM_FILE
fi


if [ ! -d "$PDB_HOME/conf" ]
         then mkdir $PDB_HOME/conf
fi
if [ ! -f "$PDB_HOME/conf/serverlist" ]
   then cp serverlist $PDB_HOME/conf/serverlist
fi

$PDB_HOME/scripts/proc/1_set_path.sh $PEM_FILE $USERNAME
$PDB_HOME/scripts/proc/2_collect_all_proc.sh $PEM_FILE $USERNAME
$PDB_HOME/scripts/proc/3_clear_temp_path.sh $PEM_FILE $USERNAME
