# x86_64（amd64）向け Ubuntu イメージを使用（M1/M2/M3 Mac 対応）
FROM --platform=linux/amd64 ubuntu:latest

# パッケージインストール（非対話 & 一括）
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

# ユーザー作成と sudo 設定
RUN adduser --disabled-password --gecos '' user && \
    echo 'user ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/user

# ユーザーに切り替え
USER user
WORKDIR /home/user
CMD ["/bin/bash"]
