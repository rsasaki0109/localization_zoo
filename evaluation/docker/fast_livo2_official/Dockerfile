FROM ros:noetic-ros-base-focal

ARG DEBIAN_FRONTEND=noninteractive
ARG FAST_LIVO2_COMMIT=0d2c0346107b75b59934975adec9a6eeeb913c64
ARG RPG_VIKIT_COMMIT=6c886c8e5d83997806e00294826d528cea3581dd
ARG SOPHUS_COMMIT=a621ff

COPY sophus-eigen3.patch /tmp/sophus-eigen3.patch

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    git \
    libboost-thread-dev \
    libeigen3-dev \
    libopencv-dev \
    libpcl-dev \
    python3-catkin-tools \
    ros-noetic-cv-bridge \
    ros-noetic-eigen-conversions \
    ros-noetic-image-transport \
    ros-noetic-pcl-ros \
    ros-noetic-tf \
    time \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/strasdat/Sophus.git /tmp/Sophus \
    && git -C /tmp/Sophus checkout "${SOPHUS_COMMIT}" \
    && git -C /tmp/Sophus apply /tmp/sophus-eigen3.patch \
    && cmake -S /tmp/Sophus -B /tmp/Sophus/build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /tmp/Sophus/build -j2 \
    && cmake --install /tmp/Sophus/build \
    && rm -rf /tmp/Sophus

RUN mkdir -p /opt/fast_livo2_ws/src \
    && git clone https://github.com/xuankuzcr/rpg_vikit.git /opt/fast_livo2_ws/src/rpg_vikit \
    && git -C /opt/fast_livo2_ws/src/rpg_vikit checkout "${RPG_VIKIT_COMMIT}" \
    && git clone https://github.com/hku-mars/FAST-LIVO2.git /opt/fast_livo2_ws/src/FAST-LIVO2 \
    && git -C /opt/fast_livo2_ws/src/FAST-LIVO2 checkout "${FAST_LIVO2_COMMIT}"

RUN . /opt/ros/noetic/setup.sh \
    && cd /opt/fast_livo2_ws \
    && catkin_make \
       -DCMAKE_BUILD_TYPE=Release \
       -DCATKIN_ENABLE_TESTING=OFF \
       --make-args fastlivo_mapping -j2

RUN printf '%s\n' \
    '#!/usr/bin/env bash' \
    'set -e' \
    'source /opt/ros/noetic/setup.bash' \
    'source /opt/fast_livo2_ws/devel/setup.bash' \
    'exec "$@"' \
    > /usr/local/bin/fast-livo2-entrypoint \
    && chmod +x /usr/local/bin/fast-livo2-entrypoint

ENTRYPOINT ["/usr/local/bin/fast-livo2-entrypoint"]
CMD ["bash"]
