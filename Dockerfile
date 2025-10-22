# Stage 1: Build
FROM ubuntu:jammy-20240530 AS builder

# Install build dependencies (added curl for NodeSource)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js 18 (for frontend build)
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get update && apt-get install -y nodejs \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy backend and frontend source into the container
COPY backend/ /app
COPY frontend/ /app/frontend

# Make the build scripts executable
RUN chmod +x build_pistache.sh build_json.sh build_backend.sh || true

# Run the backend build scripts
RUN /bin/bash ./build_pistache.sh && /bin/bash ./build_json.sh && /bin/bash ./build_backend.sh

# Build frontend: install deps and run production build
WORKDIR /app/frontend
RUN npm ci --unsafe-perm && npm install copy-webpack-plugin --save-dev && NODE_ENV=production npm run build

# Final stage: use nginx to serve frontend static files and keep runtime small
FROM nginx:stable

# Copy pistache library and backend binary from builder
COPY --from=builder /usr/local/lib/libpistache.so /usr/local/lib/libpistache.so
COPY --from=builder /app/build/crash_server /app/crash_server

# Copy built frontend files (webpack outputs to 'dist') into nginx html folder
COPY --from=builder /app/frontend/dist /usr/share/nginx/html

# Copy custom nginx config
COPY nginx.conf /etc/nginx/nginx.conf

# Copy entrypoint script (added) and make sure backend is executable
COPY entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh && chmod +x /app/crash_server || true

RUN ldconfig || true

# Expose only frontend port (80)
EXPOSE 80

# Start backend in background and nginx in foreground
ENTRYPOINT ["/app/entrypoint.sh"]