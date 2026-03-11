.. _central_cgms:

Bluetooth: Central CGMS
#######################

.. contents::
   :local:
   :depth: 2

The Central CGMS sample demonstrates how to connect to a Continuous Glucose Monitoring Service (CGMS) peripheral and read glucose measurement values.

Requirements
************

The sample supports the following development kits:

* nrf52840dk_nrf52840
* nrf5340dk_nrf5340_cpuapp
* nrf54l15dk_nrf54l15_cpuapp

The sample also requires a device running a CGMS peripheral (for example, another development kit running the :ref:`peripheral_cgms` sample).

Overview
********

When connected, the sample discovers the CGMS service and subscribes to glucose measurement notifications.
Every glucose measurement notification that is received is printed to the terminal with the glucose value converted from IEEE 11073 SFLOAT format to mg/dL.

The sample also reads the CGM Feature and CGM Status characteristics after connection to display device capabilities and current status.

Building and running
********************

.. |sample path| replace:: :file:`samples/bluetooth/central_cgms`

.. include:: /includes/build_and_run_ns.txt

Testing
=======

After programming the sample to your development kit, you can test it by connecting to another kit that is running the :ref:`peripheral_cgms` sample.

Testing with another kit
------------------------

1. |connect_terminal_specific|
#. Reset the kit.
#. Program the other development kit with the :ref:`peripheral_cgms` sample and reset it.
#. Wait until the CGMS peripheral is detected by the central.
   In the terminal window, check for information similar to the following::

      Scanning successfully started
      Device found: Nordic Glucose Sensor (random)
      Connected to device
      Service discovery completed

#. Observe that the received glucose measurements are output in the terminal window::

      CGM Feature: Type=1, Location=1
      CGM Status: Time offset=120, Status=0x00
      Glucose notification: 120.5 mg/dL
      Glucose notification: 121.0 mg/dL
      Glucose notification: 119.5 mg/dL

Dependencies
************

This sample uses the following |NCS| libraries:

* :ref:`dk_buttons_and_leds_readme`
* :ref:`gatt_dm_readme`
* :ref:`bt_scan_readme`

In addition, it uses the following Zephyr libraries:

* :file:`include/zephyr/types.h`
* :file:`include/sys/printk.h`
* :ref:`zephyr:bluetooth_api`:

  * :file:`include/bluetooth/bluetooth.h`
  * :file:`include/bluetooth/conn.h`
  * :file:`include/bluetooth/gatt.h`
  * :file:`include/bluetooth/uuid.h`
