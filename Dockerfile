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

# Python dependencies. The image uses the same locked versions as CI so that
# generated benchmark figures and reports do not drift between rebuilds.
COPY requirements-lock.txt /tmp/requirements-lock.txt
RUN pip3 install --no-cache-dir -r /tmp/requirements-lock.txt

WORKDIR /workspace
COPY . /workspace

# Build the demo binaries (synthetic benchmark + LiDAR/multimodal fixture runners)
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc) \
      --target pcd_dogfooding synthetic_benchmark multimodal_dogfooding

# Verify build
RUN test -f build/evaluation/pcd_dogfooding && \
    test -f build/evaluation/synthetic_benchmark && \
    test -f build/evaluation/multimodal_dogfooding && echo "BUILD OK"

# Default: quick one-command demo (synthetic benchmark + committed real-data
# fixtures, writes report.html; mount /out to keep the report on the host).
# For the docs refresh pipeline instead, run:
#   docker run ... python3 evaluation/scripts/refresh_study_docs.py
CMD ["bash", "evaluation/scripts/docker_demo_entrypoint.sh", "--profile", "quick"]
