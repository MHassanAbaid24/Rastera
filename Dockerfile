# Rastera development image: the one toolchain every machine and CI uses.
FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build git ca-certificates \
        flex bison \
        llvm-18-dev clang-18 libzstd-dev zlib1g-dev libedit-dev libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

ENV PATH="/usr/lib/llvm-18/bin:${PATH}"
WORKDIR /work
