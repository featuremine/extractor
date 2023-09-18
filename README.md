# Introduction

Extractor is a high-performance real-time capable alpha research
platform with an easy interface for financial analytics.

It allows you to arrange your time series feeds and operations in a
computational graph that optimizes how your computations are executed.

# How to build

## Requirements

* Cmake
* Git
* C/C++ compiler
* Python >=3.6

```bash
apt-get install -y cmake git build-essential python3
```

* numpy
* pandas

```bash
pip install numpy pandas
```

## Clone and build

```bash
git clone --recurse-submodules https://github.com/featuremine/extractor.git && \
mkdir extractor/build && \
cd extractor/build && \
cmake .. \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTING=ON \
  -DBUILD_TOOLS=ON \
  -DBUILD_WHEEL=ON \
  -DTEST_EXTENSIONS=ON \
  -DBUILD_DOCUMENTATION=OFF \
  -DCMAKE_BUILD_TYPE=Release && \
make -j 3
```

# Testing

## Run using ctest

```bash
ctest --extra-verbose
```

# Usage instructions

- [Extractor Documentation](docs/README.md)
