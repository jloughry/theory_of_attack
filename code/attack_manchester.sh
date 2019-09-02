#!/bin/sh

lockfile="./attack_lockfile"
bit_interval=1.5
half_interval=0.75

echo "ATTACK BEGINS"

gpioctl -n 20 laser
gpioctl -c laser out

laser_off() {
  gpioctl laser 0
}

laser_on() {
  gpioctl laser 1
}

send_1() {
  laser_on
  sleep $half_interval
  laser_off
  sleep $half_interval
}

send_0() {
  laser_off
  sleep $half_interval
  laser_on
  sleep $half_interval
  # I'm not sure about this last step.
  laser_off
}

# This version of Manchester code uses conventional start bits.

start_bit() {
  echo "Sending start bit."
  laser_on
  sleep $bit_interval
  laser_off
}

# This version of Manchester code uses conventional stop bits.

stop_bit() {
  echo "Sending stop bit."
  laser_off
  sleep $bit_interval
}

binary_1() {
  echo "  Sending binary 1..."
  send_1
}

binary_0() {
  echo "  Sending binary 0..."
  send_0
}

echo "Bits are sent most significant bit first."

send_ASCII_p() {
  echo " Sending ASCII 'p'"
  start_bit
  binary_0
  binary_1
  binary_1
  binary_1
  binary_0
  binary_0
  binary_0
  binary_0
  stop_bit
}

send_ASCII_0() {
  echo " Sending ASCII '0'"
  start_bit
  binary_0
  binary_0
  binary_1
  binary_1
  binary_0
  binary_0
  binary_0
  binary_0
  stop_bit
}

send_ASCII_w() {
  echo " Sending ASCII 'w'"
  start_bit
  binary_0
  binary_1
  binary_1
  binary_1
  binary_0
  binary_1
  binary_1
  binary_1
  stop_bit
}

send_ASCII_n() {
  echo " Sending ASCII 'n'"
  start_bit
  binary_0
  binary_1
  binary_1
  binary_0
  binary_1
  binary_1
  binary_1
  binary_0
  stop_bit
}

send_ASCII_e() {
  echo " Sending ASCII 'e'"
  start_bit
  binary_0
  binary_1
  binary_1
  binary_0

  binary_0
  binary_1
  binary_0
  binary_1
  stop_bit
}

send_ASCII_d() {
  echo " Sending ASCII 'd'"
  start_bit
  binary_0
  binary_1
  binary_1
  binary_0
  binary_0
  binary_1
  binary_0
  binary_0
  stop_bit
}

# Don't use intercharacter delays with Manchester coding: synchronous.
send_ASCII_p
send_ASCII_0
send_ASCII_w
send_ASCII_n
send_ASCII_e
send_ASCII_d

# For safety:
laser_off

echo "ATTACK ENDS"
rm -f $lockfile

