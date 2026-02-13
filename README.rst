nRF Connect SDK: sdk-nrf
########################

.. contents::
   :local:
   :depth: 2

This repository contains the core of nRF Connect SDK, including subsystems,
libraries, samples, and applications.
It is also the SDK's west manifest repository, containing the nRF Connect SDK
manifest (west.yml).

secure-insulin-pump usecase
**************************
We are primarly interested in the sdk-nrf for the nrf_rpc (specifically client/server
for bluetooth low energy). A good starting point is `samples/nrf_rpc/protocols_serialization`
which contains examples for both a ble rpc client/server. The client will be implemented by
our Rust `nrf-rpc` crate. The server will use the out of the box example here. For context,
the rpc commands occur via uart. To test the rpc implementation on a laptop instead of on the 
actual device hardware, we mock uart as a posix socket. The client will issue rpc commands via
a posix socket. The server will run in a separate process. Nordic/zephyr provide the ability to
run the server application on a laptop using the [BabbleSim](https://docs.zephyrproject.org/latest/develop/test/bsim.html)
that models the hardware. UART writes will be directed to a virtual tty. Using socat, we then
forward the data sent over this interface to a posix socket. Likewise, we can fully test the dual soc
rpc client/server without the physical device.

More instructions for setting up the server can be found in `samples/nrf_rpc/protocols_serialization/server/README.rst`.

Documentation
*************

Official latest documentation at https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/index.html

For earlier versions, open the latest version and use the drop-down under the title header.
