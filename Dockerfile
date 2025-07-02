FROM fedora:42
LABEL description="Image for the Gameboy project pipeline with all dependencies pre-installed."
ENV IN_DOCKER=true

RUN dnf update -y && dnf install -y \
    gcc-c++ \
    clang \
    clang-tools-extra \
    cmake \
    ninja-build \
    git \
    diffutils \
    pkgconfig \
    raylib-devel \
    catch-devel \
    mesa-libGL-devel \
    mesa-libGLU-devel \
    libX11-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel \
    alsa-lib-devel \
    && dnf clean all
