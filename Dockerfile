FROM ubuntu:20.04

SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND=noninteractive
RUN dpkg --add-architecture i386 && \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
        cmake \
        git \
        libc6:i386 \
        make \
        wget \
        && \
    apt-get clean -y && \
    apt-get autoremove --purge -y && \
    rm -rf /var/lib/apt/lists/*

RUN ln -s /usr/bin/make /usr/bin/gmake

ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll"
ARG MPLAB_XC8_VERSION=1.45
RUN cd /tmp && \
    wget ${WGET_ARGS} https://ww1.microchip.com/downloads/en/DeviceDoc/xc8-v${MPLAB_XC8_VERSION}-full-install-linux-installer.run && \
    chmod +x xc8-v${MPLAB_XC8_VERSION}-full-install-linux-installer.run && \
    echo 'Running installer (this can take up to a minute)...' && \
    ./xc8-v${MPLAB_XC8_VERSION}-full-install-linux-installer.run \
        --mode unattended --unattendedmodeui none \
        --netservername localhost --LicenseType FreeMode --prefix /opt/microchip/xc8 && \
    rm xc8-v${MPLAB_XC8_VERSION}-full-install-linux-installer.run

ENV PATH=/opt/microchip/xc8/bin:$PATH

RUN cd /tmp && \
    git clone https://github.com/ncopa/su-exec.git && \
    cd su-exec && \
    make && \
    cp -v ./su-exec /usr/local/bin && \
    chmod +s /usr/local/bin/su-exec && \
    rm -rf /tmp/su-exec
