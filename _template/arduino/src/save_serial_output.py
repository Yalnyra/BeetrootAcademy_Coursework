"""
Record the board's serial output to a log file.

Usage:  python src/save_serial_output.py   (Ctrl+C to stop)
Set SERIAL_PATH to the same port as `monitor_port` in platformio.ini.
Close the PlatformIO monitor first: only one program can hold the port.
"""
from pathlib import Path

import serial

LOG_PATH = Path(__file__).parent / ".." / "output" / "<N.N>_output.log"  # Шлях до логу
SERIAL_PATH = "COM16"  # Має співпадати з platformio.ini: monitor_port
BAUD = 115200          # Має співпадати з platformio.ini: monitor_speed

ser = serial.Serial(SERIAL_PATH, BAUD, timeout=1)

with open(LOG_PATH, "w", encoding="utf-8") as f:
    try:
        print("Listening to serial port... Press Ctrl+C to stop.")
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode("utf-8", errors="ignore")
                print(line, end="")
                f.write(line)
                f.flush()
    except KeyboardInterrupt:
        print("\nStopped by user.")
    finally:
        ser.close()
