#!/usr/bin/env bash
# Delete all containers
docker rm $(docker ps -a -q)
# Delete all images
docker rmi -f $(docker images -q)