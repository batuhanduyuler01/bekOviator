# Stage 1: Build
FROM ubuntu:jammy-20240530 as builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the source code into the container
COPY . /app

# Make the build scripts executable
RUN chmod +x build_pistache.sh build_json.sh

# Run the build scripts using bash
RUN /bin/bash ./build_pistache.sh
RUN /bin/bash ./build_json.sh
RUN /bin/bash ./build_backend.sh

# # Stage 2: Create a smaller runtime image
FROM ubuntu:jammy-20240530

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Copy the built application from the builder stage
COPY --from=builder  /usr/local/lib/libpistache.so /usr/local/lib/libpistache.so
COPY --from=builder  /app/build/crash_server /app/crash_server
    

RUN ldconfig && \
    ldd /app/crash_server

# Set the working directory
WORKDIR /app
# Expose the application port
EXPOSE 5050
# Set the entry point for the container
ENTRYPOINT ["/app/crash_server"]