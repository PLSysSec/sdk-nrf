#!/usr/bin/env python3
"""
Simple socket monitor to view nRF RPC traffic
Connects to the Unix socket and prints all data in hex format
"""

import socket
import sys
import time

SOCKET_PATH = "/tmp/nrf_rpc_server.sock"

def hex_dump(data, prefix=""):
    """Pretty print data in hex format"""
    hex_str = " ".join(f"{b:02x}" for b in data)
    ascii_str = "".join(chr(b) if 32 <= b < 127 else "." for b in data)
    print(f"{prefix}[{len(data)} bytes] {hex_str}  |{ascii_str}|")

def main():
    print(f"Connecting to {SOCKET_PATH}...")
    
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    
    try:
        sock.connect(SOCKET_PATH)
        print(f"Connected! Monitoring traffic...\n")
        
        sock.settimeout(0.1)  # Non-blocking with timeout
        
        while True:
            try:
                data = sock.recv(4096)
                if not data:
                    print("Connection closed by server")
                    break
                
                hex_dump(data, "RX: ")
                
            except socket.timeout:
                # No data, continue
                time.sleep(0.01)
            except KeyboardInterrupt:
                print("\nDisconnecting...")
                break
                
    except ConnectionRefusedError:
        print(f"Error: Could not connect to {SOCKET_PATH}")
        print("Make sure the nRF RPC server is running first!")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
    finally:
        sock.close()
        print("Disconnected")

if __name__ == "__main__":
    main()
