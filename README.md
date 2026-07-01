# autoware_multi_object_tracker

<p align="center">
  <a href="https://www.ros.org"><img src="https://img.shields.io/badge/ROS 2-jazzy-22314e"/></a>
  <a href="https://github.com/thinking-cars/autoware_multi_object_tracker/releases/latest"><img src="https://img.shields.io/github/v/release/thinking-cars/autoware_multi_object_tracker"/></a>
  <a href="https://github.com/thinking-cars/autoware_multi_object_tracker/blob/main/LICENSE"><img src="https://img.shields.io/github/license/thinking-cars/autoware_multi_object_tracker"/></a>
  <br>
  <a href="https://github.com/thinking-cars/autoware_multi_object_tracker/actions/workflows/docker-ros.yml"><img src="https://github.com/thinking-cars/autoware_multi_object_tracker/actions/workflows/docker-ros.yml/badge.svg"/></a>
  <a href="https://github.com/thinking-cars/autoware_multi_object_tracker/actions/workflows/compose-oci.yml"><img src="https://github.com/thinking-cars/autoware_multi_object_tracker/actions/workflows/compose-oci.yml/badge.svg"/></a>
  <a href="https://thinking-cars.github.io/autoware_multi_object_tracker"><img src="https://github.com/thinking-cars/autoware_multi_object_tracker/actions/workflows/docs.yml/badge.svg"/></a>
  <a href="https://github.com/thinking-cars/autoware_multi_object_tracker/actions/workflows/consistency.yml"><img src="https://github.com/thinking-cars/autoware_multi_object_tracker/actions/workflows/consistency.yml/badge.svg"/></a>
</p>

This repository integrates the [autoware_multi_object_tracker](https://github.com/autowarefoundation/autoware_universe/tree/e18a5311e509574326ae0ff6256108326a8132d0/perception/autoware_multi_object_tracker) package for multi-object tracking from [Autoware Universe](https://github.com/autowarefoundation/autoware_universe) into the [OpenADS](https://github.com/openads-project) ecosystem, which emphasizes a modular microservice architecture. Hence, this repository is self-contained and includes only the necessary dependencies.

> [!IMPORTANT]
> This repository is a prototypical integration of the `autoware_multi_object_tracker` into [OpenADS](https://github.com/openads-project) for testing and benchmarking purposes. Thus, only necessary changes were made for integration without adopting the Autoware module to the OpenADS consistency guidelines. These adoptions will only be made in case of a full integration into OpenADS after testing and benchmarking. 

<p align="center">
  <strong>🚀 <a href="#-quick-start">Quick Start</a></strong> • <strong>💻 <a href="#-development">Development</a></strong> • <strong>📝 <a href="#-documentation">Documentation</a></strong>
</p>


## 🚀 Quick Start

1. Start a container of the pre-built runtime image.
    ```bash
    docker run --rm -it ghcr.io/thinking-cars/autoware_multi_object_tracker:latest bash
    ```
1. Inside the container, launch the pre-built nodes.
    ```bash
    ros2 launch autoware_multi_object_tracker multi_object_tracker.launch.xml
    ```

## 💻 Development

### Set up Development Environment

1. Clone the repository.
    ```bash
    git clone https://github.com/thinking-cars/autoware_multi_object_tracker.git
    ```
1. Initialize the [`.openads-dev-environment`](https://github.com/openads-project/openads-dev-environment) submodule containing development environment configuration.
    ```bash
    cd autoware_multi_object_tracker
    git submodule update --init --recursive
    ```
1. Open the repository in [Visual Studio Code](https://code.visualstudio.com).
    ```bash
    code .
    ```
1. Install the recommended VS Code extensions.
    > *Ctrl+Shift+P / Extensions: Show Recommended Extensions / Install Workspace Recommended Extensions (Cloud Download Icon)*
1. Reopen the repository in a [Dev Container](https://code.visualstudio.com/docs/devcontainers/containers).
    > *Ctrl+Shift+P / Dev Containers: Rebuild and Reopen in Container*

### Build

> *Ctrl+Shift+B*

```bash
colcon build
```

### Run Tests

> *Ctrl+Shift+P / Tasks: Run Test Task*

```bash
colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=1
colcon test
colcon test-result --verbose
```


## 📝 Documentation

Package and node interfaces are documented in the respective package READMEs listed below. Implementation details are found in the [Source Code Documentation](https://thinking-cars.github.io/autoware_multi_object_tracker).

| Package | Description |
| --- | --- |
| [autoware_multi_object_tracker](autoware_multi_object_tracker/README.md) | The ROS 2 autoware_multi_object_tracker package |

## ⚖️ Licensing

The source code in this repository is licensed under Apache-2.0, see [LICENSE](LICENSE). Container images provided by this repository may contain third-party software shipped with their own license terms.

## 🙏 Acknowledgements

This project is maintained by [Thinking Cars](https://www.thinking-cars.de). We acknowledge the work of the [Autoware](https://autoware.org/) contributors and are happy to discuss potential collaborations.
