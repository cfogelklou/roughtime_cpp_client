# roughtime_cpp_client
A simple roughtime client using libsodium for signature verification, suitable for embedding.

Uses some simple stubs for sending/receiving data from a roughtime server.

## Building and running the unit test (Linux or OSX)

```bash

git clone git@github.com:cfogelklou/roughtime_cpp_client.git --recursive
cd roughtime_cpp_client
mkdir build && cd build
cmake ..
make -j8
ctest

```

