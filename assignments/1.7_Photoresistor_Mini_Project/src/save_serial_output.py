"""
Parse the UART log into a file 
voltage (with a secondary Digital ADC axis), and save a sorted copy of the log.

Log line format (from src/main.cpp's Serial.printf calls), one line looks like:
    13:38:43.224 >  | Digital ADC : 640 | Analogue ADC: 558.000000 | \
        Analogue Analytic ADC: 484.493317 | Precision Error: 13.173241 | | Timestamp: 5940154 |
"""
import serial
from pathlib import Path

# LOG_PATH = Path(__file__).parent / ".." / "unfiltered_output.log"

LOG_PATH = Path(__file__).parent / ".." / "dummy.log"
SERIAL_PATH = "COM16"
ser = serial.Serial('COM16', 115200, timeout=1)

# Open a file to save the stream output
with open(LOG_PATH, 'w', encoding='utf-8') as f:
    try:
        print("Listening to serial port... Press Ctrl+C to stop.")
        while True:
            if ser.in_waiting > 0:
                # Read a line from the serial stream
                line = ser.readline().decode('utf-8', errors='ignore')
                
                # Write to console and file
                print(line, end='')
                f.write(line)
                
                # Ensure data is written immediately to disk
                f.flush()
    except KeyboardInterrupt:
        print("\nStopped by user.")
    finally:
        ser.close()
