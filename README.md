# kortex_driver

A `robot_interface` driver plugin for Kinova Gen3/Gen3 Lite robots via the
[Kinova Kortex API](https://github.com/Kinovarobotics/kortex).

It implements the `robot_interface::RobotDriver` interface so it can be loaded
by `robot_interface` at runtime as a shared library — no mc_rtc dependency required.

## Dependencies

- [unified_robot_interface](https://isri-aist.github.io/unified_robot_interface/index.html)
- [KortexApi](https://github.com/Kinovarobotics/Kinova-kortex2_Gen3_G3L)(2.8.0)
- [fmt](https://fmt.dev)

### Installing Kinova Kortex API

The build system can automatically fetch the Kortex API via CMake, or you can install it manually:

**Option A — Automatic**

The CMake build will automatically download and build the Kortex API if not found:

```bash
cd kortex_driver
mkdir build && cd build
cmake ..
```

**Option B — Manual Installation**

1. Download the API from [Kinova Artifactory](https://artifactory.kinovaapps.com/artifactory/generic-public/kortex/API/) or [GitHub](https://github.com/Kinovarobotics/kortex)
2. Extract to a known location (e.g., `/opt/kortex`)
3. Point CMake to it during configuration:

```bash
cmake .. -DKORTEX_ROOT_DIR=/opt/kortex
```

## Build

```bash
cd kortex_driver
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH="/path/to/install;/opt/ros/humble" \
  -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build .
cmake --install .
```

The plugin is installed to `lib/robot_driver/libRobotDriverKortex.so`.

## Robot Setup & Configuration

### Session & Authentication

The driver automatically creates TCP/UDP sessions with the robot. Authentication uses the default Kinova username/password configured on the robot.
Currently, username/password are hardcoded as default values. To change them, you would need to modify the `RobotDriverKortex::conect()` in `src/RobotDriverKortex.cpp`.

## Plugin API

The shared library exports three C symbols consumed by `robot_interface`'s plugin loader:

```cpp
void ROBOT_DRIVER_PLUGIN(std::vector<std::string> & classes);
robot_interface::RobotDriver * create(const std::string & name,
                                         const std::string & ip,
                                         const uint16_t & port);
void destroy(robot_interface::RobotDriver * ptr);
```

The driver registers as `"RobotDriverKortex"` and is configured by name in YAML.
Configure it in `robot_manager/etc/mc_rtc_kortex.yaml` under the `robot_interface` key of the
relevant robot entry:

```yaml
Robots:
  kinova:
    module: Kinova
    robot_interface:
      driver: RobotDriverKortex
      ip: "localhost"
      # port: 0          # optional, unused (RTDE uses a fixed port)
```

# TODO

- [ ] Implement custom username/password
