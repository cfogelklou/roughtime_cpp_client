# roughtime_cpp_client

[![CI](https://github.com/cfogelklou/roughtime_cpp_client/actions/workflows/ci.yml/badge.svg)](https://github.com/cfogelklou/roughtime_cpp_client/actions/workflows/ci.yml)

A simple roughtime client using libsodium for signature verification, suitable for embedding.

Uses some simple "socket" stubs for sending/receiving data from a roughtime server.

## Building and running the unit tests.

Regardless of platform, you need CMake to build the unit tests.

### Linux, macOS, etc

```bash
git clone git@github.com:cfogelklou/roughtime_cpp_client.git --recursive
cd roughtime_cpp_client
mkdir build && cd build
cmake ..
make -j8
ctest
```

### Windows + Visual Studio 20xx

```bash
git clone git@github.com:cfogelklou/roughtime_cpp_client.git --recursive
cd roughtime_cpp_client
mkdir build && cd build
explorer.exe .
```

(Now open the .sln file in Visual Studio)
