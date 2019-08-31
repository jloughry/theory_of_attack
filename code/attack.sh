#!/bin/sh

bit_interval=1.5

gpioctl -n 20 laser
gpioctl -c laser out

laser_off() {
  gpioctl laser 0
}

start_bit() {
  echo "Sending start bit."
  gpioctl laser 1
  sleep $bit_interval
  gpioctl laser 0
}

stop_bit() {
  echo "Sending stop bit."
  sleep $bit_interval
}

binary_1() {
  echo "  Sending binary 1..."
  gpioctl laser 1
  sleep $bit_interval
  gpioctl laser 0
}

binary_0() {
  echo "  Sending binary 0..."
  sleep $bit_interval
}

intercharacter_delay() {
  sleep 2
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

send_ASCII_p
intercharacter_delay
send_ASCII_0
intercharacter_delay
send_ASCII_w
intercharacter_delay
send_ASCII_n
intercharacter_delay
send_ASCII_e
intercharacter_delay
send_ASCII_d

# For safety:
laser_off
