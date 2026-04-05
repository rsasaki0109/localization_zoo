FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# System dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    libeigen3-dev \
    libpcl-dev \
    libopencv-dev \
    libgtest-dev \
    libgoogle-glog-dev \
    libgflags-dev \
    libceres-dev \
    libopenmpi-dev \
    python3 \
    python3-pip \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Python dependencies
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt

WORKDIR /workspace
COPY . /workspace

# Build benchmark binary only (skip unrelated paper implementations)
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc) --target pcd_dogfooding

# Verify build
RUN test -f build/evaluation/pcd_dogfooding && echo "BUILD OK"

# Default: run full refresh (requires dogfooding_results/ to be mounted)
CMD ["python3", "evaluation/scripts/refresh_study_docs.py"]
