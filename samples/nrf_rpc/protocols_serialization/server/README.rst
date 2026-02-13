.. _nrf_rpc_protocols_serialization_server:

nRF RPC: Protocols serialization server
#######################################

.. contents::
   :local:
   :depth: 2

The Protocols serialization server sample demonstrates how to receive remote procedure calls (RPCs) from a client device, such as one running the :ref:`Protocols serialization client <nrf_rpc_protocols_serialization_client>` sample.
The RPCs are used to control :ref:`OpenThread <ug_thread_intro>`, Bluetooth® LE, and :ref:`NFC <ug_nfc>` stacks running on the server device.
The client and server devices use the :ref:`nrfxlib:nrf_rpc` and the :ref:`nrf_rpc_uart` to communicate with each other.

Requirements
************

The sample supports the following development kits:

.. table-from-sample-yaml::

To test the sample, you also need another device running the :ref:`Protocols serialization client <nrf_rpc_protocols_serialization_client>` sample.

For testing the Bluetooth LE API serialization, you need the `nRF Connect for Mobile`_ app installed on your smartphone or tablet.

For testing the NFC API serialization, you need a smartphone or tablet that can read NFC tags.

Overview
********

The Protocols serialization server sample runs full OpenThread, Bluetooth® LE, and NFC stacks, depending on the selected configuration.
The sample exposes selected functions from these stacks over UART, using the :ref:`nrfxlib:nrf_rpc`.
The remote procedure call arguments and return values are encoded using CBOR.

Configuration
*************

|config|

Snippets
========

.. |snippet| replace:: :makevar:`server_SNIPPET`

The following snippets are available:

* ``ble`` - Enables the server part of the :ref:`Bluetooth LE RPC <ble_rpc>`.
* ``coex`` - Enables the :ref:`MPSL software coexistence <nrfxlib:mpsl_cx>` implementation.
* ``debug`` - Enables debugging features, such as :c:func:`__ASSERT()` statements and verbose logging.
* ``log_rpc`` - Enables the log backend part of the :ref:`Logging RPC <log_rpc>`.
* ``openthread`` - Enables the server part of the :ref:`OpenThread RPC <ot_rpc>`.
* ``nfc`` - Enables the server part of the :ref:`NFC RPC <nfc_rpc>`.

User interface
**************

.. tabs::

   .. group-tab:: nRF52840 DK

      Button 1:

         * When the ``log_rpc`` snippet is enabled: triggers a fatal error.
           This is used for testing the core dump feature.
         * Otherwise: not available.

   .. group-tab:: nRF54L15 and nRF54LM20 DKs

      Button 0:

         * When the ``log_rpc`` snippet is enabled: triggers a fatal error.
           This is used for testing the core dump feature.
         * Otherwise: not available.

Building and running
********************

.. |sample path| replace:: :file:`samples/nrf_rpc/protocols_serialization/server`

.. include:: /includes/build_and_run.txt

.. _protocols_serialization_server_sample_testing:

Testing
*******

After building the Protocols serialization server sample and programming it to your development kit, connect it to a second device running the :ref:`Protocol serialization client <nrf_rpc_protocols_serialization_client>` sample to test either the Bluetooth LE, OpenThread or NFC functionality.

.. note::
   When using the nRF54L15 DK or nRF54LM20 DK, do not press **Button 1** or **Button 2**.
   The GPIO pins connected to these buttons are used by the UART peripheral for communication with the client device.

.. _protocols_serialization_server_app_connection:

Connecting the client and server samples
========================================

The client and server samples use two UART peripherals.
One peripheral is used for shell and logging purposes, similarly to other applications and samples, while the other peripheral is used for sending and receiving remote procedure calls (RPCs).

.. tabs::

    .. group-tab:: nRF52840 DK

        By default, the nRF52840 DK uses the ``uart0`` peripheral for shell and logging purposes, and the ``uart1`` peripheral for sending and receiving remote procedure calls (RPCs).

        The ``uart1`` peripheral is configured to use the following pins:

        .. list-table::
           :header-rows: 1

           * - Server
             - Client
             - Function on server
           * - **P1.1**
             - **P1.2**
             - RX
           * - **P1.2**
             - **P1.1**
             - TX
           * - **P1.3**
             - **P1.4**
             - RTS (hardware flow control)
           * - **P1.4**
             - **P1.3**
             - CTS (hardware flow control)
           * - **GND**
             - **GND**
             - Ground

        To enable the communication between the client and the server devices, connect the pins on the two nRF52840 DKs using jumper wires.
        The following illustration demonstrates the pin connections:

        .. figure:: /images/ps_nrf52_connections.png
            :alt: nRF52840 DK server and client pin connections

    .. group-tab:: nRF54L15 DK

        By default, the nRF54L15 DK uses the ``uart20`` peripheral for shell and logging purposes, and the ``uart21`` peripheral for sending and receiving remote procedure calls (RPCs).

        The ``uart21`` peripheral is configured to use the following pins:

        .. list-table::
           :header-rows: 1

           * - Server
             - Client
             - Function on server
           * - **P1.9**
             - **P1.8**
             - RX
           * - **P1.8**
             - **P1.9**
             - TX
           * - **P1.11**
             - **P1.12**
             - RTS (hardware flow control)
           * - **P1.12**
             - **P1.11**
             - CTS (hardware flow control)
           * - **GND**
             - **GND**
             - Ground

        To enable the communication between the client and the server devices, connect the pins on the two nRF54L15 DKs using jumper wires.
        The following illustration demonstrates the pin connections:

        .. figure:: /images/ps_nrf54l_connections.webp
            :alt: nRF54L15 DK server and client pin connections

    .. group-tab:: nRF54LM20 DK

        By default, the nRF54LM20 DK uses the ``uart20`` peripheral for shell and logging purposes, and the ``uart21`` peripheral for sending and receiving remote procedure calls (RPCs).

        The ``uart21`` peripheral is configured to use the following pins:

        .. list-table::
           :header-rows: 1

           * - Server
             - Client
             - Function on server
           * - **P1.9**
             - **P1.8**
             - RX
           * - **P1.8**
             - **P1.9**
             - TX
           * - **P1.11**
             - **P1.12**
             - RTS (hardware flow control)
           * - **P1.12**
             - **P1.11**
             - CTS (hardware flow control)
           * - **GND**
             - **GND**
             - Ground

        To enable the communication between the client and the server devices, connect the pins on the two nRF54LM20 DKs using jumper wires.

Testing Bluetooth LE API serialization
======================================

Complete the following steps to test Bluetooth LE API serialization:

#. |connect_terminal_both|

#. Reboot both devices at the same time by pressing the **RESET** button on each DK.

#. Wait a few seconds until you see a message similar to the following on both terminal emulators:

   .. code-block:: console

      [00:00:00.842,862] <inf> nrf_rpc_host: RPC client ready


   This indicates that the communication between the devices has been initialized properly.

#. Run the following command on the client's terminal emulator to start Bluetooth LE advertising:

   .. code-block:: console

      uart:~$ bt init
      uart:~$ bt advertise on


#. Start the `nRF Connect for Mobile`_ app on your smartphone or tablet.

#. Connect to the client device from the nRF Connect app.

   The device is advertising as **Nordic_PS**.

#. Observe a message similar to the following on the client's terminal emulator:

   .. code-block:: console

      LE conn param updated: int 0x0027 lat 0 to 42

#. Send data over the RX characteristic of the UART service using the nRF Connect app.

#. Observe a message similar to the following on the client's terminal emulator:

   .. code-block:: console

      bt_nus: on_receive: Received data, handle 0, conn 0x200023c4

Testing OpenThread API serialization
====================================

Complete the following steps to test OpenThread API serialization:

#. |connect_terminal_both|

#. Reboot both devices at the same time using the **RESET** button on each DK.

#. Wait a few seconds until you see a message similar to the following on both terminal emulators:

   .. code-block:: console

      uart:~$ [00:00:03.392,517] <dbg> NRF_RPC: nrf_rpc_init: Done initializing nRF RPC module


   This indicates that the communication between the devices has been initialized properly.

#. Run the following command on the client's terminal emulator to bring up the OpenThread interface on the server device:

   .. code-block:: console

      uart:~$ ot ifconfig up
      Done
      [00:02:28.980,041] <dbg> NRF_RPC: cmd_ctx_alloc: Command context 0 allocated
      [00:02:28.980,102] <dbg> NRF_RPC: nrf_rpc_cmd_common: Sending command 0x00 from group 0x01
      [00:02:28.980,133] <dbg> nrf_rpc_uart: send: Sending frame
                                             80 00 ff 01 01 f6
      ...


#. Run the following command on the client's terminal emulator to bring up the corresponding Zephyr network interface on the client device:

   .. code-block:: console

      uart:~$ net iface up 1
      Interface 1 is up

#. Verify that the Zephyr network interface has been automatically configured with OpenThread's link-local address:

   .. code-block:: console

      uart:~$ ot ipaddr
      fe80:0:0:0:6c26:956a:813:1e33
      Done
      ...

      uart:~$ net iface  show

      Interface net0 (0x200012c8) (<unknown type>) [1]
      =========================================
      MTU       : 1280
      Flags     : NO_AUTO_START,IPv6,NO_ND,NO_MLD
      Device    : ot_rpc (0x2b748)
      IPv6 unicast addresses (max 5):
            fe80::6c26:956a:813:1e33 autoconf preferred infinite

#. Start Thread and become a leader:

   .. code-block:: console

      uart:~$ ot thread start
      Done
      ...

      uart:~$ ot state leader
      Done

#. Verify that the Zephyr network interface has automatically received OpenThread's mesh-local addresses:

   .. code-block:: console

      uart:~$ net iface show

      Interface net0 (0x200012c8) (<unknown type>) [1]
      =========================================
      MTU       : 1280
      Flags     : NO_AUTO_START,IPv6,NO_ND,NO_MLD
      Device    : ot_rpc (0x2b748)
      IPv6 unicast addresses (max 5):
            fe80::6c26:956a:813:1e33 autoconf preferred infinite
            fdde:ad00:beef:0:e503:abfd:1c8d:2664 autoconf preferred infinite meshlocal


   This happens because the client registers a notification callback for OpenThread state changes at the server device and it continuously refreshes the client's IPv6 address list when that changes on the server side.

#. Retrieve the operational dataset of the Thread network:

    .. code-block:: console

      uart:~$ ot dataset active -x
      0e080000000000000000000300000b35060004001fffe00208dead00beef00cafe0708fddead00beef00000510f7893f15a55d8adeacad288c38bf32cc030a4f70656e546872656164010240120410d48d777a474f80e61aa5680de764bd6d0c0402a0f7f8
      Done

#. To send a UDP packet to a peer device and UDP port 5555, run the following command:

   .. code-block:: console

      uart:~$ net udp send fe80:0:0:0:6c26:956a:813:1e34 5555 AAAA
      Message sent

#. To open the UDP port 5555 and listen for incoming UDP datagrams from peer devices, run the following command:

   .. code-block:: console

      uart:~$ net udp bind fe80:0:0:0:6c26:956a:813:1e33 5555

Note that all IPv6 addresses shown here are just examples and will be replaced with the actual IPv6 addresses of the peer devices.

Testing NFC API serialization
====================================

#. |connect_terminal_both|

#. Reboot both devices at the same time by pressing the **RESET** button on each DK.

#. Wait a few seconds until you see a message similar to the following on both terminal emulators:

   .. code-block:: console

      [00:00:00.842,862] <inf> nrf_rpc_host: RPC client ready


   This indicates that the communication between the devices has been initialized properly.

#. Run the following commands on the client's terminal emulator to bring up NFC interface on the server device:

   .. code-block:: console

      uart:~$ nfc init
      uart:~$ nfc start

#. Start any application on smartphone or tablet that is able to read NFC tags.

#. Touch the NFC antenna with the smartphone or tablet and observe it displays the encoded text ``Hello world!``.

#. Run the following commands on the client's terminal emulator to set and encode a custom message on the server device:

   .. code-block:: console

      uart:~$ nfc stop
      uart:~$ nfc set_new_msg "https://www.nordicsemi.com/Products/Development-software/nRF-Connect-SDK"
      uart:~$ nfc start

#. Touch the NFC antenna with the smartphone or tablet and observe it displays the custom message as the encoded text.

#. Run the following commands on the client's terminal emulator to stop and to release NFC frontend on the server device:

   .. code-block:: console

      uart:~$ nfc stop
      uart:~$ nfc release

Dependencies
************

This sample uses the following `sdk-nrfxlib`_ libraries:

* :ref:`nrfxlib:nrf_rpc`
* :ref:`nrfxlib:nfc_api_type2`

Server Simulation Guide and Setup
*********************************
This runs the nrf_rpc ble server implementation on your laptop. 

Requirements:
- West
- BabbleSim
- socat
 
West can be installed via: `pip3 install west` (perhaps best in venv).

BabbleSim mocks up the nrf hardware on your laptop. To set this up, follow (this guide)[https://docs.zephyrproject.org/latest/boards/native/nrf_bsim/doc/nrf52_bsim.html].
You may not need to export the variables at the end of the above guide. Test it is working with:

```
$ west build -b nrf52_bsim samples/hello_world
$ ./build/zephyr/zephyr.exe -nosim
Press Ctrl+C to exit
```

Socat is a tool that forwards our bytestream to our posix socket (allowing simulated insulin pump soc to communicate with simulated ble stack soc).

Build the server binary:
```
# command executed in server directory
west build --build-dir build . --board nrf52_bsim -- -DCONF_FILE="prj_native_sim.conf"
```

Now you need to run the generated executable. We need to pass some args and setup the simulator
so we have a commmand `./run_bsim.sh`

This will give output of the form:

```
Starting BabbleSim PHY simulator...
Starting time monitor device...
Starting nRF RPC server with BabbleSim...

=== BabbleSim Running ===
PHY PID: 26115
Monitor PID: 26119
Simulation ID: nrf_rpc_test
Simulation length: 86400 seconds (24 hours simulated, ~39 seconds real time at 2200x speed)

To test RX, run in another terminal:
  socat UNIX-LISTEN:/tmp/nrf_rpc_server.sock,fork /dev/pts/XX,raw,echo=0
  printf '\x04\x00\xff\x00\xff\x00\x62\x74\x5f\x72\x70\x63' | socat - UNIX-CONNECT:/tmp/nrf_rpc_server.sock

Starting device (Press Ctrl+C to stop)...

d_00: @00:00:00.000000  UART 0 (UARTE0) connected to pseudotty: /dev/pts/4
d_00: @00:00:00.000000  *** Booting nRF Connect SDK v3.2.1-77c5ea27ba7d ***
d_00: @00:00:00.000000  *** Using Zephyr OS v4.2.99-ec78104f1569 ***
d_00: @00:00:00.000000  [00:00:00.000,000] <inf> nrf_ps_server: Initializing RPC server
d_00: @00:00:00.000000  [00:00:00.000,000] <dbg> NRF_RPC: Initializing nRF RPC module
d_00: @00:00:00.000000  [00:00:00.000,000] <dbg> NRF_RPC: Group 'bt_rpc' has id 0
d_00: @00:00:00.000000  [00:00:00.000,000] <dbg> NRF_RPC: Group 'rpc_utils' has id 1
d_00: @00:00:00.000000  [00:00:00.000,000] <dbg> NRF_RPC: Done initializing nRF RPC module
d_00: @00:00:00.000000  [00:00:00.000,000] <dbg> nrf_rpc_uart: init called
d_00: @00:00:00.000000  [00:00:00.000,000] <dbg> nrf_rpc_uart: <<< TX packet 0c1a
d_00: @00:00:00.000000                                         04 00 ff 00 ff 00 62 74  5f 72 70 63             |......bt _rpc    
d_00: @00:00:00.001350  [00:00:00.001,342] <dbg> nrf_rpc_uart: <<< TX packet 4251
d_00: @00:00:00.001350                                         04 00 ff 01 ff 00 72 70  63 5f 75 74 69 6c 73    |......rp c_utils 
```

Now we need to map our pseudotty to a posix socket using socat. You will need to check what /dev/ptys/XX
you are attached to. For the above output: we see `UART 0 (UARTE0) connected to pseudotty: /dev/pts/4`

Using this run (no output expected):
```
socat UNIX-LISTEN:/tmp/nrf_rpc_server.sock,fork,reuseaddr /dev/pts/XX,raw,echo=0
```

Finally, we now send data to the socket using `./test_rpc.sh`. 


