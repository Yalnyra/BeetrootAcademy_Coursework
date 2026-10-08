"""
Parse the ESP32 ADC precision-error log, plot Precision Error vs measured
voltage (with a secondary Digital ADC axis), and save a sorted copy of the log.

Log line format (Only valid if src/main.cpp's is built without TELEPLOT_ON preprocessor flag)
    Timestamp: 1516000 | Unfiltered Digital ADC: 3590 | Filtered Digital ADC: 3586 | Trigger Output:  1 | Trigger Floor: 2200 | Trigger Ceiling: 3000

"""

from pathlib import Path
import matplotlib.pyplot as plt

LOG_PATH = Path(__file__).parent / ".." / "output.log"
SORTED_LOG_PATH = Path(__file__).parent / ".." / "sorted_output.log"
FIGURE_PATH = Path(__file__).parent / ".." / "adc_error_vs_voltage.png"

ADC_RESOLUTION_BITS = 12
ADC_MAX = 4095          # matches ADC_MAX in main.cpp
U_REF_MV = 3100.0       # matches U_REF in main.cpp
THRESHOLD_DARK = 1800; 
THRESHOLD_LIGHT = 3000;

REQUIRED_FIELDS = {"Timestamp", "Unfiltered Digital ADC", "Filtered Digital ADC", "Trigger Output", "Trigger Floor", "Trigger Ceiling"}

def parse_log(path: Path) -> list[dict]:
    """Read the log file and return one dict of {label: value_string} per line."""
    records = []
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line_number, line in enumerate(f, start=1):
            chunks = line.split("|")  # Include all fields, since the timestamp is required for plotting
            record = {}
            for chunk in chunks:
                chunk = chunk.strip()
                if not chunk or ":" not in chunk:
                    continue  # the empty "| |" segment lands here, cleanly skipped
                label, value = chunk.split(":", 1)
                record[label.strip()] = value.strip()

            missing = REQUIRED_FIELDS - record.keys()
            if missing:
                print(f"Skipping line {line_number}: missing {missing}")
                continue

            records.append(record)

    return records


def get_timestamp(d):
    return int(d["Timestamp"])


def get_digital_adc(d):
    return int(d["Unfiltered Digital ADC"])

def get_adc_filtered(d):
    return int(d["Filtered Digital ADC"])

def get_ctrl_input(d):
    return int(d["Trigger Output"])


def digital_to_voltage(digital):
    """Same linear mapping main.cpp uses for rstAnalogValueAnalytic."""
    return (digital / ADC_MAX) * U_REF_MV


def voltage_to_digital(voltage):
    return (voltage / U_REF_MV) * ADC_MAX


def plot_error_vs_voltage(records: list, path: Path):
    
    ts = [get_timestamp(r) for r in records]
    analog_adc_raw = [get_digital_adc(r) for r in records]
    analog_adc_filtered = [get_adc_filtered(r) for r in records]
    ctrl_input = [get_ctrl_input(r) for r in records]
    voltage_ceil = float(records[0]['Trigger Ceiling'])
    voltage_floor = float(records[0]['Trigger Floor'])
    fig, ax = plt.subplots(3, 1, figsize=(9, 6), sharex=False)


    ax[0].plot(ts, analog_adc_raw,  color="#FF6600", alpha=0.75)
    ax[0].set_xlabel("ESP Clock time (milliSeconds)")
    ax[0].set_ylabel("Raw ADC Voltage")
    ax[0].set_title("Unfiltered Photoresistor voltage")
    ax[0].grid(True, color="#DDDDDD", linewidth=0.6)
    ax[0].axhline(voltage_ceil, linestyle='--', color="#2C2929")
    ax[0].axhline(voltage_floor, linestyle='--', color="#2C2929")
    # secax = ax.secondary_xaxis("top", functions=(voltage_to_digital, digital_to_voltage))
    # secax.set_xlabel(f"Digital ADC code (0-{ADC_MAX}, {ADC_RESOLUTION_BITS}-bit)")

    ax[1].plot(ts, analog_adc_filtered, color="#FF6600", alpha=0.75)
    ax[1].set_xlabel("ESP Clock time (milliSeconds)")
    ax[1].set_ylabel("Filtered ADC Voltage")
    ax[1].set_title("Exponential MA of Photoresistor voltage")
    ax[1].grid(True, color="#DDDDDD", linewidth=0.6)
    
    ax[1].axhline(voltage_ceil, linestyle='--', color="#2C2929")
    ax[1].axhline(voltage_floor, linestyle='--', color="#2C2929")

    ax[2].plot(ts, ctrl_input,  color="#006600", alpha=0.75)
    ax[2].set_xlabel("ESP Clock time (milliSeconds)")
    ax[2].set_ylabel("Relay (Closed=0/Open=1)")
    ax[2].set_title("Exponential MA of Photoresistor voltage")
    ax[2].grid(True, color="#DDDDDD", linewidth=0.6)
    ax[2].set_axisbelow(True)


    fig.tight_layout()
    fig.savefig(path, dpi=150)
    print(f"Saved figure to {path.resolve()}")
    return fig


def save_sorted_log(records, path: Path):
    """Write records sorted by Timestamp descending, one line per record,
    using the same field labels ("keys") the original log used."""
    sorted_records = sorted(records, key=get_timestamp, reverse=False)

    # fields = ["Timestamp", "Unfiltered Digital ADC", "Filtered Digital ADC", "Trigger Output", "Trigger Floor", "Trigger Ceiling"]

    with open(path, "w", encoding="utf-8") as f:
        for record in sorted_records:
            line = " | ".join(f"{field}: {record[field]}" for field in REQUIRED_FIELDS)
            f.write(f"| {line} |\n")

    print(f"Saved {len(sorted_records)} sorted record(s) to {path.resolve()}")
    return sorted_records


if __name__ == "__main__":
    records = parse_log(LOG_PATH)
    
    sorted_records = save_sorted_log(records, SORTED_LOG_PATH)
    print(f"Parsed {len(records)} record(s) from {LOG_PATH.resolve()}")
    plot_error_vs_voltage(sorted_records, FIGURE_PATH)
