#!/usr/bin/env python3
import serial
import sys
import binascii

# HDLC constants
HDLC_FLAG = 0x7E
HDLC_ESC  = 0x7D
HDLC_XOR  = 0x20

def hdlc_encode(data: bytes) -> bytes:
    """HDLC-encode the payload."""
    encoded = bytearray()
    encoded.append(HDLC_FLAG)
    for b in data:
        if b in (HDLC_FLAG, HDLC_ESC):
            encoded.append(HDLC_ESC)
            encoded.append(b ^ HDLC_XOR)
        else:
            encoded.append(b)
    encoded.append(HDLC_FLAG)
    return bytes(encoded)

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} /dev/pts/N")
        sys.exit(1)

    port = sys.argv[1]
    print(f"Opening serial port: {port}")

    # Example payload: your nRF RPC group init packet
    payload = bytes([0x04, 0x00, 0xff, 0x00, 0xff, 0x00, 0x62, 0x74, 0x5f, 0x72, 0x70, 0x63])

    encoded = hdlc_encode(payload)
    print(f"HDLC encoded payload: {binascii.hexlify(encoded)}")

    try:
        ser = serial.Serial(port, 115200, timeout=0.1)
    except Exception as e:
        print(f"Failed to open {port}: {e}")
        sys.exit(1)

    print(f"Sending payload...")
    ser.write(encoded)
    ser.flush()
    print("Sent!")

    # Optional: read responses
    try:
        while True:
            data = ser.read(64)
            if data:
                print(f"RX ({len(data)} bytes): {binascii.hexlify(data)}")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        ser.close()

if __name__ == "__main__":
    main()

