#!/bin/bash
# Test script to send RPC binding and see the response

echo "Sending bt_rpc binding packet and capturing response..."
printf '\x04\x00\xff\x00\xff\x00\x62\x74\x5f\x72\x70\x63' | socat - UNIX-CONNECT:/tmp/nrf_rpc_server.sock | hexdump -C

echo ""
echo "If you see output above with '62 74 5f 72 70 63' (bt_rpc) and '72 70 63 5f 75 74 69 6c 73' (rpc_utils),"
echo "then the server is responding with both binding acknowledgments!"
