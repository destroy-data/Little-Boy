FROM alpine:latest
LABEL description="Image for the Gameboy project pipeline with all dependencies pre-installed."

RUN apk update && apk add --no-cache \
    clang \
    clang-extra-tools \
    clang-dev \
    cmake \
    ninja \
    git \
    mesa-dev \
    mesa-gbm \
    mesa-egl \
    libstdc++-dev

