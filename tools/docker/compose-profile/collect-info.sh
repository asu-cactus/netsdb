#!/usr/bin/env bash

# turn off the profiler with SIGINT
docker-compose kill -s SIGINT

# make the directory for profile info
mkdir ../../../profile-info

sleep 2

for i in $(docker-compose ps -q) ; do
  docker cp $i:/profile-info  ../../../profile-info/$i
done
