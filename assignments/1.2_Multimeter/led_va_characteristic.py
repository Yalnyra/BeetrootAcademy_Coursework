#!/usr/bin/env python3
"""
led_va_characteristic.py

Plot the forward Volt-Ampere (I-V) characteristic of an LED from a CSV
table of bench measurements.

Expected measurement setup
---------------------------
A variable supply drives a series resistor + LED. For each measurement
point you record:
  - the forward voltage across the LED under load (V)
  - the forward current through the LED under load (mA)
  - the resistance of the series resistor used for that point (ohm)

Only the forward-conduction region is assumed to be present in the data
(no reverse-bias / breakdown points), so the script simply sorts by
voltage and plots current vs. voltage.

Expected CSV format
--------------------
A header row plus one row per measurement, e.g.:

    Voltage_V,Current_mA,Resistance_Ohm
    1.50,0.02,4700
    1.65,0.08,4700
    1.80,0.35,1000
    1.95,2.10,1000
    2.05,6.40,470
    ...

Column names don't have to match exactly - the script looks for columns
whose header contains "volt"/"v", "curr"/"ma"/"i", or "resist"/"ohm"/"r"
(case-insensitive). If auto-detection picks the wrong column, or your
headers don't contain any recognizable keyword, override it explicitly
with --v-col / --i-col / --r-col (pass the exact header text).

Usage
-----
    pip install pandas matplotlib
    python led_va_characteristic.py measurements.csv
    python led_va_characteristic.py measurements.csv -o iv_curve.png
    python led_va_characteristic.py measurements.csv --log
    python led_va_characteristic.py measurements.csv --color-by-resistor
    python led_va_characteristic.py measurements.csv \
        --v-col "Vf (V)" --i-col "If (mA)" --r-col "R (ohm)"
"""

import argparse
import re
import sys
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


def find_column(columns, keywords):
    """Return the first column whose header matches one of the keyword
    patterns (case-insensitive), or None if nothing matches."""
    for col in columns:
        normalized = col.strip().lower()
        for kw in keywords:
            if re.search(kw, normalized):
                return col
    return None


def resolve_columns(df, v_col, i_col, r_col):
    columns = list(df.columns)

    if v_col is None:
        v_col = find_column(columns, [r"volt", r"\bvf\b", r"\bu\b", r"\bv\b"])
    if i_col is None:
        i_col = find_column(columns, [r"curr", r"\bif\b", r"\bma\b", r"\bi\b"])
    if r_col is None:
        r_col = find_column(columns, [r"resist", r"ohm", r"\br\b"])

    missing = [
        name
        for name, val in (("voltage", v_col), ("current", i_col))
        if val is None
    ]
    if missing:
        sys.exit(
            "Could not auto-detect column(s) for: {}\n"
            "Available columns: {}\n"
            "Re-run with --v-col / --i-col (and --r-col if needed) to "
            "specify them explicitly.".format(", ".join(missing), columns)
        )
    if v_col not in columns or i_col not in columns or (r_col and r_col not in columns):
        sys.exit(
            "One of the specified columns was not found.\n"
            "Available columns: {}".format(columns)
        )

    return v_col, i_col, r_col


def main():
    parser = argparse.ArgumentParser(
        description="Plot the forward V-A (I-V) characteristic of an LED "
        "from a CSV of bench measurements.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("csv_path", type=Path, help="Path to the input CSV file")
    parser.add_argument(
        "-o", "--output", type=Path, default=None,
        help="Output image path (default: <csv_path stem>_iv_curve.png next to the CSV)",
    )
    parser.add_argument(
        "--log", action="store_true",
        help="Plot current on a logarithmic axis (useful for seeing the "
        "exponential diode region across several decades of current)",
    )
    parser.add_argument(
        "--color-by-resistor", action="store_true",
        help="Color-code each point by the series resistor value used for "
        "that measurement (adds a colorbar); requires a resistance column",
    )
    parser.add_argument("--v-col", default=None, help="Exact header name of the voltage column")
    parser.add_argument("--i-col", default=None, help="Exact header name of the current column")
    parser.add_argument("--r-col", default=None, help="Exact header name of the resistance column")
    parser.add_argument("--no-show", action="store_true", help="Don't open an interactive window, just save the file")
    args = parser.parse_args()

    if not args.csv_path.exists():
        sys.exit(f"Input file not found: {args.csv_path}")

    df = pd.read_csv(args.csv_path)
    v_col, i_col, r_col = resolve_columns(df, args.v_col, args.i_col, args.r_col)

    if args.color_by_resistor and r_col is None:
        sys.exit(
            "--color-by-resistor was requested but no resistance column "
            "could be found/resolved. Pass --r-col explicitly."
        )

    # Keep only rows with valid numeric voltage/current, forward region
    # (V > 0, I >= 0), and sort by voltage so the connecting line is monotonic.
    data = df[[c for c in (v_col, i_col, r_col) if c is not None]].copy()
    data = data.apply(pd.to_numeric, errors="coerce")
    data = data.dropna(subset=[v_col, i_col])
    data = data[(data[v_col] > 0) & (data[i_col] >= 0)]
    data = data.sort_values(v_col).reset_index(drop=True)

    if data.empty:
        sys.exit("No valid forward-region measurement rows found after parsing the CSV.")

    fig, ax = plt.subplots(figsize=(8, 6))

    if args.color_by_resistor:
        # Thin connecting line in a neutral color, points colored by R.
        ax.plot(data[v_col], data[i_col], "-", color="#999999", linewidth=1.2, zorder=1)
        scatter = ax.scatter(
            data[v_col], data[i_col],
            c=data[r_col], cmap="viridis",
            s=45, edgecolors="white", linewidths=0.6, zorder=2,
        )
        cbar = fig.colorbar(scatter, ax=ax)
        cbar.set_label("Series resistor, R (Ω)")
    else:
        ax.plot(
            data[v_col], data[i_col],
            marker="o", markersize=5.5, linewidth=1.8,
            color="#1f77b4", markerfacecolor="#1f77b4",
        )

    if args.log:
        ax.set_yscale("log")

    ax.set_xlabel("Forward voltage, $V_F$ (V)")
    ax.set_ylabel("Forward current, $I_F$ (mA)")
    ax.set_title("LED Volt-Ampere (I-V) Characteristic — Forward Region")
    ax.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.6)
    ax.set_xlim(left=0)
    if not args.log:
        ax.set_ylim(bottom=0)

    fig.tight_layout()

    output_path = args.output or args.csv_path.with_name(args.csv_path.stem + "_iv_curve.png")
    fig.savefig(output_path, dpi=300)
    print(f"Saved plot to {output_path}")
    print(f"Points plotted: {len(data)}")
    if r_col is not None:
        unique_r = sorted(data[r_col].dropna().unique())
        print(f"Series resistor value(s) in data: {unique_r} ohm")

    if not args.no_show:
        plt.show()


if __name__ == "__main__":
    main()
