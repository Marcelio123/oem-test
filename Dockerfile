FROM ubuntu:20.04

# Avoid interactive prompts during package install
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpthread-stubs0-dev \
    && rm -rf /var/lib/apt/lists/*

# Create work directory
WORKDIR /app

# Copy everything into the container
COPY . .

# Create build directory and build
RUN mkdir build && cd build && cmake .. && make

# Set the default command
CMD ["./build/gx_test"]
