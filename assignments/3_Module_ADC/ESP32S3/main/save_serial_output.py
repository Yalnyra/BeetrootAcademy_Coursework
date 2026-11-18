"""
Capture serial output or parse ESP-IDF monitor log files to plain-text.

Modes
-----
Live capture (original behaviour, now with CLI args):
    python save_serial_output.py --port COM16 --output log.txt
    python save_serial_output.py -p COM16 -b 115200 -o log.txt

Parse an ESP-IDF log file saved by Ctrl+L / idf_monitor:
    python save_serial_output.py --input log.3_Module_ADC.20261118134957.txt
    python save_serial_output.py -i log.3_Module_ADC.20261118134957.txt -o parsed.txt

Output format (parse mode)
--------------------------
Each line is stripped of ANSI escape sequences so it matches what the raw
serial port would have produced without CONFIG_LOG_COLORS.  A session header
carries the capture timestamp extracted from the filename, and each line is
prefixed with that same timestamp so the log is self-contained:

    # source : log.3_Module_ADC.20261118134957.txt
    # captured: 2026-11-18 13:49:57
    [2026-11-18 13:49:57] I (70266) mini_project_3: SPEED_K=2, ...
    [2026-11-18 13:49:57] W (78266) encoder: COUNT INCREASED BY 1
    [2026-11-18 13:49:57] E (83216) encoder: COUNT DECREASED BY -1
"""

import argparse
import re
import sys
from datetime import datetime
from typing import Optional
from pathlib import Path

try:
    import serial
    _HAS_SERIAL = True
except ImportError:
    _HAS_SERIAL = False

# ---------------------------------------------------------------------------
# ANSI helpers
# ---------------------------------------------------------------------------

ANSI_RE = re.compile(r'\x1b\[[0-9;]*[mGKHFJA-Z]')

# Map ANSI colour code -> single-char ESP-IDF log level (for reference)
_ANSI_LEVEL = {
    '0;32': 'I',   # green          -> Info
    '0;33': 'W',   # yellow         -> Warning
    '1;31': 'E',   # bold red       -> Error
    '0;34': 'D',   # blue           -> Debug
    '0;35': 'V',   # purple/magenta -> Verbose
}


def strip_ansi(text: str) -> str:
    return ANSI_RE.sub('', text)


# ---------------------------------------------------------------------------
# Filename timestamp
# ---------------------------------------------------------------------------

def capture_time_from_filename(filename: str) -> Optional[datetime]:
    """
    Extract the capture datetime from filenames like:
        log.NAME.YYYYMMDDHHMMSS.txt
    Returns a datetime object or None.
    """
    m = re.search(r'(\d{14})', filename)
    if m:
        try:
            return datetime.strptime(m.group(1), '%Y%m%d%H%M%S')
        except ValueError:
            pass
    return None


# ---------------------------------------------------------------------------
# Mode 1 - live serial capture
# ---------------------------------------------------------------------------

def live_capture(port: str, baud: int, output: Path) -> None:
    if not _HAS_SERIAL:
        sys.exit("pyserial is not installed.  Run: pip install pyserial")

    print(f"Opening {port} @ {str(baud)} baud ...  Ctrl+C to stop")
    print(f"Logging to: {output}")

    ser = serial.Serial(port, baud, timeout=1)
    with open(output, 'w', encoding='utf-8') as f:
        try:
            while True:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore')
                    print(line, end='')
                    f.write(line)
                    f.flush()
        except KeyboardInterrupt:
            print("\nStopped by user.")
        finally:
            ser.close()

    print(f"Saved -> {output}")


# ---------------------------------------------------------------------------
# Mode 2 - parse ESP-IDF monitor log
# ---------------------------------------------------------------------------

def parse_idf_log(input_path: Path, output: Path) -> None:
    cap_ts = capture_time_from_filename(input_path.name)
    ts_str = cap_ts.strftime('%Y-%m-%d %H:%M:%S') if cap_ts else 'unknown'
    prefix = f'[{ts_str}] '

    header_lines = [
        f'# source  : {input_path.name}',
        f'# captured: {ts_str}',
        f'# format  : stripped ANSI -- matches raw serial output',
        '',
    ]

    print('\n'.join(header_lines))

    lines_written = 0
    with open(input_path, 'r', encoding='utf-8', errors='ignore') as fin, \
         open(output,     'w', encoding='utf-8')                       as fout:

        for h in header_lines:
            fout.write(h + '\n')

        for raw in fin:
            # Strip ANSI escape codes -> plain "I (12345) tag: msg"
            clean = strip_ansi(raw).rstrip('\r\n')
            if not clean:
                continue

            out_line = prefix + clean + '\n'
            print(out_line, end='')
            fout.write(out_line)
            lines_written += 1

    print(f"\n{lines_written} lines -> {output}")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def auto_output(args) -> Path:
    """Return a sensible default output path when --output is not given."""
    if args.input:
        stem = args.input.stem + '.parsed'
        return args.input.parent / (stem + '.txt')
    # live capture: serial_COM16_20261118134957.txt next to the script
    ts = datetime.now().strftime('%Y%m%d%H%M%S')
    return Path(f'serial_{args.port}_{ts}.txt')


def main() -> None:
    parser = argparse.ArgumentParser(
        description='Capture serial output or parse an ESP-IDF monitor log.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )

    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--port',  '-p', metavar='COMx',
                      help='Serial port for live capture (e.g. COM16)')
    mode.add_argument('--input', '-i', metavar='FILE', type=Path,
                      help='ESP-IDF monitor log file to parse')

    parser.add_argument('--output', '-o', metavar='FILE', type=Path,
                        help='Output file (default: auto-named alongside input / cwd)')
    parser.add_argument('--baud',   '-b', type=int, default=115200,
                        help='Baud rate for live capture (default: 115200)')

    args = parser.parse_args()
    out  = args.output or auto_output(args)

    if args.port:
        live_capture(args.port, args.baud, out)
    else:
        if not args.input.exists():
            sys.exit(f"Input file not found: {args.input}")
        parse_idf_log(args.input, out)


if __name__ == '__main__':
    main()
