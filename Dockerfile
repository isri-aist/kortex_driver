# Runs `uri interface` with the RobotDriverKortex plugin (Universal Robots).
#
# Built on the unified_robot_interface image (ROS Jazzy, mc_rtc, zenoh and
# URI already installed in ${URI_PREFIX}=/opt/uri), so only this driver is
# built here. It is installed in the same prefix, where `uri interface` looks
# for driver plugins.
ARG BASE_IMAGE=ghcr.io/isri-aist/unified_robot_interface:latest
FROM ${BASE_IMAGE}

USER root

COPY . /tmp/kortex_driver
RUN . /opt/ros/${ROS_DISTRO}/setup.sh \
    && cmake -S /tmp/kortex_driver -B /tmp/kortex_driver/build \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_PREFIX_PATH="${URI_PREFIX};${EXTRA_DEPS_PREFIX};/opt/ros/${ROS_DISTRO}" \
        -DCMAKE_INSTALL_PREFIX=${URI_PREFIX} \
    && cmake --build /tmp/kortex_driver/build --parallel $(nproc) \
    && cmake --install /tmp/kortex_driver/build \
    && mkdir -p /config ${URI_PREFIX}/share \
    && cp -r /tmp/kortex_driver/etc ${URI_PREFIX}/share/kortex_driver \
    && rm -rf /tmp/kortex_driver \
    && test -f ${URI_PREFIX}/lib/robot_interface/libRobotDriverKortex.so

USER vscode
WORKDIR /config

# Mount a directory holding robot_interface.yaml on /config.
# Example configs are in /opt/uri/share/kortex_driver.
# The base image's entrypoint sources /opt/ros/${ROS_DISTRO}/setup.bash.
CMD ["uri", "interface", "-c", "/config/robot_interface.yaml"]

