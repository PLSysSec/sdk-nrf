#!/bin/bash
# BabbleSim test setup for nRF RPC UART

# Set BabbleSim paths
export BSIM_OUT_PATH=/home/tp/ncs/v3.2.1/tools/bsim
export BSIM_COMPONENTS_PATH=${BSIM_OUT_PATH}/components
export LD_LIBRARY_PATH=${BSIM_OUT_PATH}/lib:${LD_LIBRARY_PATH}

# Simulation ID (use same for all devices in the simulation)
SIM_ID="nrf_rpc_test"

# Device number
DEVICE_NUM=0

# Clean up old lock files
rm -rf /tmp/bs_${USER}/${SIM_ID} 2>/dev/null

echo "Starting BabbleSim PHY simulator..."
cd ${BSIM_OUT_PATH}/bin
./bs_2G4_phy_v1 -s=${SIM_ID} -D=2 -sim_length=86400e6 &
PHY_PID=$!

# Wait for PHY to start
sleep 1

# Start time monitor to advance simulation time (suppress output)
echo "Starting time monitor device..."
./bs_device_time_monitor -s=${SIM_ID} -d=1 -interval=10000000 >/dev/null 2>&1 &
MONITOR_PID=$!

sleep 0.5

echo "Starting nRF RPC server with BabbleSim..."
cd /home/tp/ncs/v3.2.1/nrf/samples/nrf_rpc/protocols_serialization/server/build/server/zephyr
./zephyr.exe -s=${SIM_ID} -d=${DEVICE_NUM} -uart0_pty -uart_pty_pollT=1000 &
SERVER_PID=$!

sleep 1

echo ""
echo "=== BabbleSim Running ===" 
echo "PHY PID: ${PHY_PID}"
echo "Monitor PID: ${MONITOR_PID}"
echo "Server PID: ${SERVER_PID}"
echo "Simulation ID: ${SIM_ID}"
echo "Simulation length: 86400 seconds (24 hours simulated, ~39 seconds real time at 2200x speed)"
echo "Pseudo-TTY should be shown in server output above"
echo ""
echo "To test RX, run in another terminal:"
echo "  socat UNIX-LISTEN:/tmp/nrf_rpc_server.sock,fork /dev/pts/XX,raw,echo=0"
echo "  printf '\\x04\\x00\\xff\\x00\\xff\\x00\\x72\\x70\\x63\\x5f\\x75\\x74\\x69\\x6c\\x73' | socat - UNIX-CONNECT:/tmp/nrf_rpc_server.sock"
echo ""
echo "Waiting for server process to complete..."

# Wait for server to exit
wait ${SERVER_PID}
SERVER_EXIT=$?
echo ""
echo "Server exited with code: ${SERVER_EXIT}"
