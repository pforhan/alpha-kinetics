FROM cubanismo/jaguar-sdk:latest

# Switch to archived Debian repositories (buster is EOL) 
# Switch to modern Ubuntu 24.04 base to valid Glibc compatibility with Thorsten Otto's tools
FROM ubuntu:24.04

# Avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    wget \
    unzip \
    xz-utils \
    ca-certificates \
    libgmp-dev \
    libmpc-dev \
    libmpfr-dev \
    libncurses5-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Thorsten Otto's Cross-Tools (GCC 7.5.0) for m68k-atari-mint
# We install to /opt/cross-mint to keep it isolated
RUN mkdir -p /opt/cross-mint

# Binutils 2.45
RUN wget -qO- https://tho-otto.de/download/mint/binutils-2.45-mint-20250812-bin-linux64.tar.xz | tar -C /opt/cross-mint -xJ

# GCC 7.5.0 (Compatible with libraries and modern glibc 2.39 in Noble)
RUN wget -qO- https://tho-otto.de/download/mint/gcc-7.5.0-mint-20230719-bin-linux64.tar.xz | tar -C /opt/cross-mint -xJ

# MintLib 0.60.1
RUN wget -qO- https://tho-otto.de/download/mint/mintlib-0.60.1-mint-20240718-dev.tar.xz | tar -C /opt/cross-mint -xJ

# PML 2.03
RUN wget -qO- https://tho-otto.de/download/mint/pml-2.03-mint-20171006-dev.tar.xz | tar -C /opt/cross-mint -xJ

# Add Cross-Compiler to PATH
ENV PATH="/opt/cross-mint/usr/bin:${PATH}"

# Install RMAC 2.4.12 (Assembler)
RUN wget -q -O /tmp/rmac.zip http://rmac.is-slick.com/static/rmac-2.4.12-lin-x64.zip && \
    unzip /tmp/rmac.zip -d /tmp/rmac_extract && \
    # Find the binary (it might be in a subdir) and move it
    find /tmp/rmac_extract -name rmac -type f -exec cp {} /usr/local/bin/ \; && \
    chmod +x /usr/local/bin/rmac && \
    rm -rf /tmp/rmac.zip /tmp/rmac_extract

# Install RLN 1.7.4 (Linker)
RUN wget -q -O /tmp/rln.zip http://rmac.is-slick.com/static/rln-1.7.4-lin64.zip && \
    unzip /tmp/rln.zip -d /tmp/rln_extract && \
    find /tmp/rln_extract -name rln -type f -exec cp {} /usr/local/bin/ \; && \
    chmod +x /usr/local/bin/rln && \
    rm -rf /tmp/rln.zip /tmp/rln_extract

# Verify installation (rmac/rln don't support --version, so we check path presence)
RUN which rmac && which rln

WORKDIR /workspaces/jag-kinetics
