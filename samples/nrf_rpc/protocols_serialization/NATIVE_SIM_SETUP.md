# nRF RPC Native Simulation Setup

This setup allows you to run both the client and server on your laptop using Zephyr's native_sim platform with IPC (Inter-Process Communication) instead of UART.

## Prerequisites

- nRF Connect SDK v3.2.1 or later
- Linux environment (native_sim works best on Linux)

## Architecture

The native_sim configuration uses:
- **IPC Service** with **ICMSG backend** for inter-process communication
- **Shared memory** region for data exchange between client and server
- **MBOX** (mailbox) for signaling between processes

Both client and server applications run as separate native Linux processes and communicate through shared memory.

## Building

### Build Server
```bash
cd /home/tp/ncs/v3.2.1/nrf/samples/nrf_rpc/protocols_serialization/server
west build -b native_sim -p auto -- -DCONF_FILE=prj_native_sim.conf
```

### Build Client
```bash
cd /home/tp/ncs/v3.2.1/nrf/samples/nrf_rpc/protocols_serialization/client
west build -b native_sim -p auto -- -DCONF_FILE=prj_native_sim.conf
```

## Running

You need to run both applications simultaneously in separate terminals.

### Terminal 1: Run Server
```bash
cd /home/tp/ncs/v3.2.1/nrf/samples/nrf_rpc/protocols_serialization/server
./build/zephyr/zephyr.exe
```

### Terminal 2: Run Client
```bash
cd /home/tp/ncs/v3.2.1/nrf/samples/nrf_rpc/protocols_serialization/client
./build/zephyr/zephyr.exe
```

## Expected Behavior

- Both processes will start and establish communication via IPC
- You'll see debug logs showing nRF RPC initialization and communication
- The shell will be available in both processes for issuing commands
- Commands sent from client will be executed on server via RPC

## Debugging

### Enable Verbose Logging
Already enabled in `prj_native_sim.conf`:
- `CONFIG_NRF_RPC_LOG_LEVEL_DBG=y`
- `CONFIG_NRF_RPC_TR_LOG_LEVEL_DBG=y`
- `CONFIG_NRF_RPC_OS_LOG_LEVEL_DBG=y`
- `CONFIG_NRF_RPC_CBOR_LOG_LEVEL_DBG=y`

### Common Issues

1. **Port already in use**: If you see IPC errors, make sure no other instances are running
2. **Shared memory issues**: The shared memory region at `0x20070000` must be accessible to both processes
3. **Timing issues**: Make sure to start the server first, then the client

## Advantages of Native Sim

- **Fast iteration**: No flashing required, instant restart
- **Easy debugging**: Use standard Linux debugging tools (gdb, valgrind, etc.)
- **Development speed**: Test RPC logic without hardware
- **CI/CD friendly**: Can run in automated testing environments

## Files Created

- `server/prj_native_sim.conf` - Server configuration for native_sim
- `client/prj_native_sim.conf` - Client configuration for native_sim  
- `server/boards/native_sim.overlay` - Server device tree overlay for IPC
- `client/boards/native_sim.overlay` - Client device tree overlay for IPC

## Next Steps

To revert to hardware (nRF52840DK), simply build without the `-DCONF_FILE` option:
```bash
west build -b nrf52840dk_nrf52840 -p auto
```
