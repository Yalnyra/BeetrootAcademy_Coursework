#!/usr/bin/env python3
"""
led_va_characteristic.py

Plot the forward Volt-Ampere (I-V) characteristic of an LED from a CSV
table of bench measurements.

Two input formats are supported (auto-detected from the file, or pick
explicitly with --format):

1. "generic" - a simple header row with voltage/current/(resistance)
   columns, comma-separated. See the docstring of resolve_columns() /
   the example below.

2. "beetroot" - the semicolon-delimited multimeter-log format used for
   the 1.2_Multimeter assignment: one row per reading of
       V_s ; R ; V_r ; V_f ; I_f ; led_color ; led_id
   (source voltage, series-resistor resistance, voltage across that
   resistor, LED forward voltage, forward current in mA, LED color,
   LED unique id). I_f == -1 marks "no current measurement was taken"
   for that row and is excluded from the plot. led_id may be blank.

Generic format example
-----------------------
A header row plus one row per measurement, e.g.:

    Voltage_V,Current_mA,Resistance_Ohm
    1.50,0.02,4700
    1.65,0.08,4700
    1.80,0.35,1000
    ...

Column names don't have to match exactly - the script looks for columns
whose header contains "volt"/"v", "curr"/"ma"/"i", or "resist"/"ohm"/"r"
(case-insensitive). If auto-detection picks the wrong column, or your
headers don't contain any recognizable keyword, override it explicitly
with --v-col / --i-col / --r-col (pass the exact header text).

Beetroot multimeter-log format example
----------------------------------------
    V_s; V_r; V_f; I_f; led_color; led_id;
    5.; 10.; 1.1; 3.89; -1; white; 1;
    5.; 220; 2.09; 2.86; 8.99; white; 1;
    ...
(Note the real file's header is missing the resistance column name -
this script parses the beetroot format by fixed field position, not by
header text, so that mismatch doesn't matter.)

Only the forward-conduction region is assumed to be present in the data
(no reverse-bias / breakdown points).

Usage
-----
    pip install pandas matplotlib

    # generic format
    python led_va_characteristic.py measurements.csv
    python led_va_characteristic.py measurements.csv -o iv_curve.png
    python led_va_characteristic.py measurements.csv --log
    python led_va_characteristic.py measurements.csv --color-by-resistor
    python led_va_characteristic.py measurements.csv \
        --v-col "Vf (V)" --i-col "If (mA)" --r-col "R (ohm)"

    # beetroot multimeter-log format
    python led_va_characteristic.py LED_Measurements.csv
    python led_va_characteristic.py LED_Measurements.csv --colors white
    python led_va_characteristic.py LED_Measurements.csv --colors white,blue
    python led_va_characteristic.py LED_Measurements.csv --colors white,blue
"""

import argparse
import csv
import re
import sys
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Approximate (V_F, I_F) points tracing the shape of each LED color's curve
# in the reference chart the user supplied. That chart's curve axis isn't
# numerically calibrated (no voltage tick labels), so these are a
# by-eye reading of the curve's shape anchored to the chart's labeled
# typical V_F range (white: 3.0-5.0 V) and its 0-50 mA current axis - a
# reasonable approximation, not a precise datasheet digitization.
REFERENCE_CURVES = {
    "white": [
        (3.0, 0.0),
        (3.2, 0.3),
        (3.4, 1.5),
        (3.6, 5.0),
        (3.8, 12.0),
        (4.0, 22.0),
        (4.2, 35.0),
        (4.4, 45.0),
        (4.6, 50.0),
    ],
}


# ---------------------------------------------------------------------
# Generic format
# ---------------------------------------------------------------------

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


def plot_generic(args):
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


# ---------------------------------------------------------------------
# Beetroot multimeter-log format
# ---------------------------------------------------------------------

def _to_float(text):
    text = text.strip()
    if text in ("", "-", "N/A", "NA", "n/a"):
        return float("nan")
    try:
        return float(text)
    except ValueError:
        return float("nan")


def parse_beetroot_csv(path):
    """Parse the semicolon-delimited multimeter-log format by fixed
    field position: V_s ; R ; V_r ; V_f ; I_f ; led_color ; led_id

    The real file's header only names 6 columns (it's missing the
    resistance column) while every data row actually carries 7 values,
    so header text is not used to identify columns here - position is.
    A row with only 6 values is missing led_id, which is kept as "".
    """
    rows = []
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        reader = csv.reader(f, delimiter=";")
        next(reader, None)  # header row (unreliable field names - skip)
        for lineno, raw in enumerate(reader, start=2):
            cells = [c.strip() for c in raw]
            while cells and cells[-1] == "":
                cells.pop()  # drop the empty cell left by a trailing ';'
            if not cells:
                continue
            if len(cells) not in (6, 7):
                print(
                    f"Warning: line {lineno} has {len(cells)} fields "
                    f"(expected 6 or 7) - skipping: {raw}",
                    file=sys.stderr,
                )
                continue
            v_s, r_ohm, v_r, v_f, i_f, led_color = cells[:6]
            led_id = cells[6] if len(cells) == 7 else ""
            rows.append({
                "v_source_V": _to_float(v_s),
                "r_ohm": _to_float(r_ohm),
                "v_resistor_V": _to_float(v_r),
                "v_led_V": _to_float(v_f),
                "i_led_mA": _to_float(i_f),
                "led_color": led_color.strip().lower(),
                "led_id": led_id.strip(),
            })
    return pd.DataFrame(rows)


def _id_sort_key(led_id):
    # Numeric ids sort numerically; a blank/non-numeric id sorts last.
    try:
        return (0, int(led_id))
    except (ValueError, TypeError):
        return (1, led_id)


def plot_beetroot(df, colors, output_path, no_show):
    total_selected = int(df["led_color"].isin(colors).sum())
    subset = df[df["led_color"].isin(colors)].copy()

    na_mask = subset["i_led_mA"] == -1
    excluded_na = int(na_mask.sum())
    subset = subset[~na_mask]
    subset = subset.dropna(subset=["v_led_V", "i_led_mA"])

    if subset.empty:
        sys.exit(
            "No valid rows left for color(s) {} after excluding N/A current "
            "readings.".format(", ".join(colors))
        )

    fig, ax = plt.subplots(figsize=(8, 6))

    # One line per individual LED (led_id), not aggregated - each id's
    # readings are sorted by voltage and connected, so different physical
    # units stay visually distinct instead of being pooled into one curve.
    groups = []
    for color in colors:
        color_subset = subset[subset["led_color"] == color]
        for led_id in sorted(color_subset["led_id"].unique(), key=_id_sort_key):
            grp = color_subset[color_subset["led_id"] == led_id].sort_values("v_led_V")
            if grp.empty:
                continue
            groups.append((color, led_id, grp))

    rainbow = plt.cm.rainbow(np.linspace(0, 1, max(len(groups), 1)))
    x_candidates = [subset["v_led_V"].min(), subset["v_led_V"].max()]

    for (color, led_id, grp), line_color in zip(groups, rainbow):
        id_label = led_id if led_id else "?"
        ax.plot(
            grp["v_led_V"], grp["i_led_mA"],
            marker="o", markersize=6, linewidth=1.8, color=line_color,
            zorder=3, label=f"{color.title()} LED #{id_label} (measured)",
        )

    # Reference curve(s): an approximate trace of the datasheet chart's
    # curve shape for each plotted color that has one (see REFERENCE_CURVES).
    for color in colors:
        ref_points = REFERENCE_CURVES.get(color)
        if not ref_points:
            continue
        ref_v, ref_i = zip(*ref_points)
        ax.plot(
            ref_v, ref_i, linestyle="--", color="#333333", linewidth=1.8,
            zorder=2, label=f"{color.title()} (reference, approx.)",
        )
        x_candidates += list(ref_v)

    # Dashed 20 mA reference line (a common "rated current" mark for small LEDs).
    ax.axhline(20, color="#999999", linestyle="--", linewidth=1.2, zorder=1)
    ax.annotate(
        "20 mA", xy=(max(x_candidates) + 0.25, 20), xytext=(0, 3),
        textcoords="offset points", fontsize=8, color="#666666",
        ha="right", va="bottom",
    )

    # Current axis: 50 mA (the reference chart's bound) unless a measured
    # point runs higher, in which case the axis grows to fit it.
    y_max = max(50, subset["i_led_mA"].max() * 1.08)
    ax.set_ylim(0, y_max)
    ax.set_xlim(max(0, min(x_candidates) - 0.3), max(x_candidates) + 0.3)

    ax.set_xlabel("Forward voltage, $V_F$ (V)")
    ax.set_ylabel("Forward current, $I_F$ (mA)")
    color_label = " & ".join(c.title() for c in colors)
    ax.set_title(f"LED Volt-Ampere (I-V) Characteristic — {color_label} LED(s), Forward Region")
    ax.grid(True, linestyle="--", linewidth=0.5, alpha=0.6)
    ax.legend(loc="upper left", fontsize=8.5)
    fig.tight_layout()

    fig.savefig(output_path, dpi=300)
    print(f"Saved plot to {output_path}")
    print(
        f"Rows for color(s) {', '.join(colors)}: {total_selected}; "
        f"excluded as N/A current: {excluded_na}; plotted points: {len(subset)}"
    )

    # Bonus sanity check: directly-measured I_F vs. V_r / R for the same
    # row. Large disagreement flags a possibly mis-recorded reading; this
    # doesn't change what's plotted, it's just printed for your review.
    check = subset.dropna(subset=["v_resistor_V", "r_ohm"])
    check = check[check["r_ohm"] != 0]
    if not check.empty:
        print("\nSanity check (measured I_F vs. V_r / R, in mA):")
        for _, row in check.iterrows():
            computed_mA = (row["v_resistor_V"] / row["r_ohm"]) * 1000.0
            id_label = row["led_id"] or "?"
            print(
                f"  id={id_label:<3} color={row['led_color']:<6} "
                f"R={row['r_ohm']:>8.1f} ohm   measured I_F={row['i_led_mA']:>6.2f} mA   "
                f"V_r/R={computed_mA:>6.2f} mA"
            )

    if not no_show:
        plt.show()


# ---------------------------------------------------------------------
# Format detection + CLI
# ---------------------------------------------------------------------

def detect_format(path):
    with open(path, "r", encoding="utf-8-sig") as f:
        first_line = f.readline()
    if first_line.count(";") > first_line.count(","):
        return "beetroot"
    return "generic"


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
        "--format", choices=["auto", "generic", "beetroot"], default="auto",
        help="Input format. 'auto' (default) picks 'beetroot' for a "
        "semicolon-delimited file, 'generic' otherwise.",
    )

    generic_group = parser.add_argument_group("generic format options")
    generic_group.add_argument(
        "--log", action="store_true",
        help="Plot current on a logarithmic axis (generic format only)",
    )
    generic_group.add_argument(
        "--color-by-resistor", action="store_true",
        help="Color-code each point by the series resistor value used for "
        "that measurement, with a colorbar (generic format only)",
    )
    generic_group.add_argument("--v-col", default=None, help="Exact header name of the voltage column")
    generic_group.add_argument("--i-col", default=None, help="Exact header name of the current column")
    generic_group.add_argument("--r-col", default=None, help="Exact header name of the resistance column")

    beetroot_group = parser.add_argument_group("beetroot format options")
    beetroot_group.add_argument(
        "--colors", default="white",
        help="Comma-separated LED color(s) to include, e.g. 'white' or "
        "'white,blue' (beetroot format only; default: white)",
    )
    parser.add_argument("--no-show", action="store_true", help="Don't open an interactive window, just save the file")
    args = parser.parse_args()

    if not args.csv_path.exists():
        sys.exit(f"Input file not found: {args.csv_path}")

    fmt = args.format
    if fmt == "auto":
        fmt = detect_format(args.csv_path)

    if fmt == "beetroot":
        df = parse_beetroot_csv(args.csv_path)
        colors = [c.strip().lower() for c in args.colors.split(",") if c.strip()]
        output_path = args.output or args.csv_path.with_name(args.csv_path.stem + "_iv_curve.png")
        plot_beetroot(df, colors, output_path, args.no_show)
    else:
        plot_generic(args)


if __name__ == "__main__":
    main()
