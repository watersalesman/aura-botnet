# Aura Botnet - Bot Client

Geting Started
---
The client executable or script need to be run in the same folder as
any necessary components (systemd services for Linux clients and launcher.vbs
for Powershell) on the target machine, or installed via web delivery (see
below).

You can choose whichever client you feel is most appropriate for the platform
and use case, although specified above, the C++ client is the first priority
when it comes to features and stability.

### C++ Client
There are three header-only projects being used for this client. Thanks to the
following projects for their awesome work:
* [okdshin/PicoSHA2](https://github.com/okdshin/PicoSHA2/) -
A header-file-only, SHA256 hash generator in C++
* [nlohmann/json](https://github.com/nlohmann/json/) -
A JSON library for Modern C++
* [catchorg/Catch2](https://github.com/catchorg/Catch2/) -
A modern, C++-native, header-only, test framework for unit-tests, TDD and BDD

For the Linux C++ client, you will also need *libcurl* installed with
development files. There may be a *libcurl-devel* or *libcurl-dev* package on
your distro that you need to install.

Afterward, change into the source directory ([`aura-client`](../aura-client)).
Then build by running:
```
cmake .    # Run with "-DCMAKE_BUILD_TYPE=Debug" to build tests
make
```
Tests require a local httpbin server running at port 5000. Find more
information [here](https://httpbin.org/).

For building the Windows C++ client, you can find various methods of using
CMake on Windows
[here](http://preshing.com/20170511/how-to-build-a-cmake-based-project/).

Catch2 unit tests can be found under [`aura-client/tests/`](../aura-client/tests/).
Do not run them in parallel. They rely on constant filenames and may interfere
with each other.

### Rust Client
For the Rust client, you will need Rust installed on the platform that you wish
to compile for. Unfortunately, there is no simple method of cross-compiling that
I am aware of. Go into the source directory ([`alt-clients/rust/rust-linux`](../alt-clients/rust/rust-linux) or
[`alt-clients/rust/rust-windows`](../alt-clients/rust/rust-windows) and run:
```
cargo build --release
```
The binary will be sent to `alt-clients/rust/rust-*/target/release/`.

### Powershell and Bash
The Powershell and Bash clients simply need to be executed on the target
machine.
