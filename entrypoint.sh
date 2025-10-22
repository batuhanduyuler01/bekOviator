#!/bin/sh
set -e

# Ensure ld cache is updated for libpistache
ldconfig || true

# Start the backend (assumes /app/crash_server exists and is executable)
if [ -x /app/crash_server ]; then
  /app/crash_server &
  BACKEND_PID=$!
  echo "Started backend (pid $BACKEND_PID)"
else
  echo "Warning: /app/crash_server not found or not executable"
fi

# Forward signals to backend and stop gracefully
_term() {
  echo "Caught SIGTERM, stopping..."
  if [ -n "$BACKEND_PID" ]; then
    kill -TERM "$BACKEND_PID" 2>/dev/null || true
    wait "$BACKEND_PID" || true
  fi
  exit 0
}

trap _term TERM INT

# Start nginx in foreground (nginx provided by the image)
exec nginx -g "daemon off;"
