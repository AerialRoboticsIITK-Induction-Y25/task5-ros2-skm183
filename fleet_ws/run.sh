#!/bin/bash

set -e

docker build --no-cache -t ariitk_drone_fleet -f Dockerfile ..

echo "=== Launching Drone Fleet ==="
docker run -it --rm \
  --net=host \
  -v $(pwd)/../part1:/workspace/part1 \
  -v $(pwd)/src:/workspace/fleet_ws/src \
  ariitk_drone_fleet
