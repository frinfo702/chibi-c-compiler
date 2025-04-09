FROM ubuntu:latest

# パッケージのインストールとクリーンアップを1つのレイヤーにまとめる
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    gcc \
    make \
    git \
    binutils \
    libc6-dev \
    gdb \
    sudo && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# ユーザー設定を1つのレイヤーにまとめる
RUN adduser --disabled-password --gecos '' user && \
    echo 'user ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/user

USER user
WORKDIR /home/user
RUN mkdir -p /home/user/work
WORKDIR /home/user/work
