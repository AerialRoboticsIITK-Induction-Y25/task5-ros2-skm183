FROM ros:jazzy AS builder

WORKDIR /workspace

COPY part1/ ./part1/
COPY fleet_ws/ ./fleet_ws/

WORKDIR /workspace/fleet_ws
RUN /bin/bash -c "source /opt/ros/jazzy/setup.bash && colcon build"

FROM ros:jazzy

WORKDIR /workspace/fleet_ws

COPY --from=builder /workspace/fleet_ws/install ./install

RUN echo '#!/bin/bash' > /entrypoint.sh && \
    echo 'source /opt/ros/jazzy/setup.bash' >> /entrypoint.sh && \
    echo 'source /workspace/fleet_ws/install/setup.bash' >> /entrypoint.sh && \
    echo 'exec "$@"' >> /entrypoint.sh && \
    chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

CMD ["ros2", "launch", "drone_fleet", "fleet.launch.py"]
