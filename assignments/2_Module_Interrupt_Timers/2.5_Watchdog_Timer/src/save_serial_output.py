"""
Parse the Serial log into a file 

"""
import serial
from pathlib import Path

LOG_PATH = Path(__file__).parent / ".." / "output" / "2.7_output.log"
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
