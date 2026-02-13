#!/bin/bash
# Bridge the native_sim UART pseudo-TTY to a Unix socket

SOCKET_PATH="/tmp/nrf_rpc_server.sock"
PTY_DEVICE=""

# Remove old socket if it exists
rm -f "$SOCKET_PATH"

echo "Waiting for server to start and create pseudo-TTY..."
echo "Start ./build/server/zephyr/zephyr.exe in another terminal first"
echo ""

# Wait for the PTY to be created and extract it
for i in {1..30}; do
    if [ -n "$PTY_DEVICE" ]; then
        break
    fi
    sleep 0.5
done

if [ -z "$PTY_DEVICE" ]; then
    echo "ERROR: Could not detect pseudo-TTY device"
    echo "Please provide the PTY path manually (e.g., /dev/pts/13):"
    read PTY_DEVICE
fi

if [ ! -e "$PTY_DEVICE" ]; then
    echo "ERROR: Device $PTY_DEVICE does not exist"
    exit 1
fi

echo "Bridging $PTY_DEVICE <-> $SOCKET_PATH"
echo "Clients can connect to: $SOCKET_PATH"
echo ""

# Bridge the PTY to the Unix socket
# Clients connecting to the socket will communicate with the UART
exec socat UNIX-LISTEN:$SOCKET_PATH,fork,reuseaddr $PTY_DEVICE,raw,echo=0
