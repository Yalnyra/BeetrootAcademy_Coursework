"""
Parse the ESP32 ADC precision-error log, plot Precision Error vs measured
voltage (with a secondary Digital ADC axis), and save a sorted copy of the log.

Log line format (from src/main.cpp's Serial.printf calls), one line looks like:
    13:38:43.224 >  | Digital ADC : 640 | Analogue ADC: 558.000000 | \
        Analogue Analytic ADC: 484.493317 | Precision Error: 13.173241 | | Timestamp: 5940154 |

Known quirks in the raw log, confirmed by inspecting output.log directly:
  - A serial-monitor timestamp is glued onto the front of every line.
  - A stray empty "| |" segment sits before "Timestamp" (a blank Serial.printf
    call in the firmware's loop()).
  - At least one line has "Timestamp:5941946" with no space after the colon
    (a dropped byte during USB-CDC capture) -- confirmed via
    `grep -c Timestamp output.log` == 347 == total lines, so every line DOES
    have the field; it's a formatting quirk, not missing data.
Splitting on the bare "|" character and stripping each side of ":" survives
all three quirks; splitting on the literal " | " / ": " (with spaces) does not
-- that was the bug behind the missing/garbled keys we were chasing earlier.
"""

from pathlib import Path
import matplotlib.pyplot as plt

LOG_PATH = Path(__file__).parent / ".." / "output.log"
SORTED_LOG_PATH = Path(__file__).parent / ".." / "sorted_output.log"
FIGURE_PATH = Path(__file__).parent / ".." / "adc_error_vs_voltage.png"

ADC_RESOLUTION_BITS = 12
ADC_MAX = 4095          # matches ADC_MAX in main.cpp
U_REF_MV = 3100.0       # matches U_REF in main.cpp

REQUIRED_FIELDS = {"Digital ADC", "Analogue ADC", "Analogue Analytic ADC", "Precision Error", "Timestamp"}


def parse_log(path: Path) -> list[dict]:
    """Read the log file and return one dict of {label: value_string} per line."""
    records = []
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line_number, line in enumerate(f, start=1):
            chunks = line.split("|")[1:]  # chunk[0] is the serial-monitor timestamp, not a field
            record = {}
            for chunk in chunks:
                chunk = chunk.strip()
                if not chunk or ":" not in chunk:
                    continue  # the stray empty "| |" segment lands here, cleanly skipped
                label, value = chunk.split(":", 1)
                record[label.strip()] = value.strip()

            missing = REQUIRED_FIELDS - record.keys()
            if missing:
                print(f"Skipping line {line_number}: missing {missing}")
                continue

            records.append(record)

    return records


def get_digital_adc(d):
    return int(d["Digital ADC"])


def get_analogue_adc(d):
    return float(d["Analogue ADC"])


def get_precision_error(d):
    return float(d["Precision Error"])


def digital_to_voltage(digital):
    """Same linear mapping main.cpp uses for rstAnalogValueAnalytic."""
    return (digital / ADC_MAX) * U_REF_MV


def voltage_to_digital(voltage):
    return (voltage / U_REF_MV) * ADC_MAX


def plot_error_vs_voltage(records, path: Path):
    analog_mv = [get_analogue_adc(r) for r in records]
    error_pct = [get_precision_error(r) for r in records]

    fig, ax = plt.subplots(figsize=(9, 6))
    ax.scatter(analog_mv, error_pct, s=14, color="#3366CC", alpha=0.75, edgecolors="none")

    ax.set_xlabel("Analogue ADC voltage (mV)")
    ax.set_ylabel("Precision error (%)")
    ax.set_title("ADC precision error vs. measured voltage")
    ax.grid(True, color="#DDDDDD", linewidth=0.6)
    ax.set_axisbelow(True)

    secax = ax.secondary_xaxis("top", functions=(voltage_to_digital, digital_to_voltage))
    secax.set_xlabel(f"Digital ADC code (0-{ADC_MAX}, {ADC_RESOLUTION_BITS}-bit)")

    fig.tight_layout()
    fig.savefig(path, dpi=150)
    print(f"Saved figure to {path.resolve()}")
    return fig


def save_sorted_log(records, path: Path):
    """Write records sorted by Digital ADC descending, one line per record,
    using the same field labels ("keys") the original log used."""
    sorted_records = sorted(records, key=get_digital_adc, reverse=True)

    fields = ["Digital ADC", "Analogue ADC", "Analogue Analytic ADC", "Precision Error", "Timestamp"]
    with open(path, "w", encoding="utf-8") as f:
        for record in sorted_records:
            line = " | ".join(f"{field}: {record[field]}" for field in fields)
            f.write(f"| {line} |\n")

    print(f"Saved {len(sorted_records)} sorted record(s) to {path.resolve()}")
    return sorted_records


if __name__ == "__main__":
    records = parse_log(LOG_PATH)
    print(f"Parsed {len(records)} record(s) from {LOG_PATH.resolve()}")

    plot_error_vs_voltage(records, FIGURE_PATH)
    save_sorted_log(records, SORTED_LOG_PATH)
