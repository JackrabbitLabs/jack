# Overview

Jack is a CLI tool that implements the CXL 2.0 Fabric Management API 
specification.  It is intended to be used to configure and monitor CXL 
compliant hardware devices such as switches, accelerators or memory controllers.

# Supported Operating System Versions

- Ubuntu 23.10
- Fedora 38, 39

> Note: Ubuntu 22.04 is not supported. This is due to some newer PCI features that
> are missing from the 5.15 Linux kernel that ships with Ubuntu 22.04.

# Building

1. Install OS libraries

Install the following build packages to compile the software on the following
operating systems.

**Ubuntu:**

```bash
apt install build-essential libglib2.0-dev libyaml-dev libpci-dev
```

**Fedora:**

```bash
```

2. Build Dependencies

To clone and build dependencies run:

```bash
./builddeps.bash
```

3. Build

After building the required dependencies run:

```bash
make
```

# Usage

Jack connects to a target device using MCTP over TCP. When using Jack with an
endpoint such as 
[CSE (CXL Switch Emulator)](https://github.com/JackrabbitLabs/cse), the user 
first starts the CSE application using a config file that defines a 
virtualized CXL switch environment with the following command.

```bash
cse -lc config.yaml
```

Once the target endpoint is running, Jack can be used to query or configure
the CXL endpoint.

# Example Commands

To obtain identity information about the endpoint:

```bash
jack show id
```

To obtain information about the capabilities of the endpoint:

```bash
jack show switch
```

To show the status of the ports of the endpoint use `show port`. This displays
the devices that are connected to the endpoint.

```bash
jack show port
```

To show information about what ports are connected to a Virtual CXL Switch
(VCS).

```bash
jack show vcs 0
```

To unbind a port (or a Logical Device) from a VCS:

```bash
jack port unbind -c 0 -b 4
```

To bind a port (or a Logical Device) to a VCS:

```bash
jack port bind -p 4 -l 0 -c 0 -b 4
```

