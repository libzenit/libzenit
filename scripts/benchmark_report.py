#!/usr/bin/env python3
#
#    LibZenit
#    Copyright (C) 2026  Ian Torres
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License version 3
#    as published by the Free Software Foundation.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
"""
Benchmark report generator for LibZenit.

Parses CTest benchmark output from multiple CI environments and produces:
  - BENCHMARK.md       — markdown report with tables and embedded charts
  - benchmark_charts/   — PNG chart images referenced by the report

Usage:
  python scripts/benchmark_report.py                                        \
      --env "macOS (Apple Silicon)" --file benchmark_macos.log              \
      --env "Linux ARM64 (gcc)"       --file benchmark_arm.log               \
      --env "Linux x86_64 (gcc)"     --file benchmark_ubuntu.log

Each log file should contain raw CTest output (the full log from the
"Run benchmarks" step).  The script extracts lines matching the
zenit_bench_print format.

New benchmarks are auto-categorised by name prefix — update PREFIX_MAP
when adding a new module so it gets its own chart category.
"""

import argparse
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CHARTS_DIR = REPO_ROOT / "benchmark_charts"
REPORT_PATH = REPO_ROOT / "BENCHMARK.md"

# Matches:  name                      1000000 iters    0.1234 s    8101234 ops/s
BENCH_LINE_RE = re.compile(
    r'^\s*(?P<name>\S+)\s+'
    r'(?P<iters>\d+)\s+iters\s+'
    r'(?P<time>[\d.]+)\s+s\s+'
    r'(?P<ops>[\d.]+)\s+ops/s'
)

# Override mapping for benchmark names whose auto-detected category
# (the first segment before '_') needs a friendlier display name.
# Most benchmarks are auto-categorised — add an entry here only when
# the default looks wrong.
CATEGORY_OVERRIDE = {
    "libzenit": "Version",
    "malloc": "malloc (baseline)",
}


def categorize(bench_name: str) -> str:
    """Auto-detect category from the first segment of the benchmark name.

    A benchmark named ``ring_seq_128B`` becomes category ``Ring``,
    ``map_insert`` → ``Map``, etc.  Overrides in CATEGORY_OVERRIDE
    take precedence.
    """
    segment = bench_name.split("_")[0] if "_" in bench_name else bench_name
    return CATEGORY_OVERRIDE.get(segment, segment.capitalize())


def build_categories(data: dict) -> dict[str, list[str]]:
    """Group benchmark names by auto-detected category."""
    cats: dict[str, list[str]] = {}
    for bench in data:
        cat = categorize(bench)
        cats.setdefault(cat, []).append(bench)
    return cats


def parse_log(text: str):
    """Extract benchmark results from raw CTest log text.

    The CTest output prefixes each benchmark line with 'N: ' (e.g. '1: '),
    which is stripped before matching.

    Returns a dict: {benchmark_name: (iters, time_s, ops_per_sec)}
    """
    results = {}
    for line in text.splitlines():
        clean = line
        if ":" in line:
            parts = line.split(":", 1)
            if len(parts) == 2 and parts[0].strip().isdigit():
                clean = parts[1]
        m = BENCH_LINE_RE.match(clean)
        if m:
            name = m.group("name")
            iters = int(m.group("iters"))
            time_s = float(m.group("time"))
            ops = float(m.group("ops"))
            results[name] = (iters, time_s, ops)
    return results


def build_data(environments: list[tuple[str, dict]]):
    """Build a structured dict from parsed environments.

    Returns: {bench_name: {env_name: (iters, time_s, ops)}}
    """
    all_benches = set()
    for _, results in environments:
        all_benches.update(results.keys())

    data = {}
    for bench in sorted(all_benches):
        data[bench] = {}
        for env_name, results in environments:
            data[bench][env_name] = results.get(bench, (0, 0.0, 0.0))
    return data


def fmt_ops(ops: float) -> str:
    """Format ops/sec with SI suffix."""
    for unit in ("", "K", "M", "B"):
        if ops < 1000:
            return f"{ops:,.0f} {unit}ops/s" if unit == "" else f"{ops:,.2f} {unit}ops/s"
        ops /= 1000
    return f"{ops:,.2f} Tops/s"


# ── chart generation ──────────────────────────────────────────────────────

HAS_MPL = False
try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.ticker as mticker
    HAS_MPL = True
except ImportError:
    pass


def _chart_path(name: str) -> Path:
    CHARTS_DIR.mkdir(parents=True, exist_ok=True)
    return CHARTS_DIR / name


def _plot_grouped_bars(data: dict, env_names: list[str], bench_names: list[str],
                       colors: list, log_scale: bool, figsize: tuple,
                       font_size: int, rot: int) -> tuple:
    """Draw a grouped bar chart of ops/sec for the given benchmarks.

    Returns (fig, ax) for the caller to finalise.
    """
    n_envs = len(env_names)
    width = 0.8 / n_envs
    x = range(len(bench_names))

    fig, ax = plt.subplots(figsize=figsize)
    for i, env in enumerate(env_names):
        vals = [data[b][env][2] for b in bench_names]
        offset = (i - (n_envs - 1) / 2) * width
        bars = ax.bar([p + offset for p in x], vals, width,
                      label=env, color=colors[i])
        for bar, v in zip(bars, vals):
            if v > 0:
                ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height(),
                        fmt_ops(v).replace(" ops/s", ""),
                        ha="center", va="bottom", fontsize=font_size, rotation=rot)

    if log_scale:
        ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels(bench_names, rotation=30, ha="right", fontsize=9)
    ax.legend(fontsize=8)
    ax.grid(axis="y", alpha=0.3)
    fig.tight_layout()
    return fig, ax


def _chart_categories(data: dict, env_names: list[str],
                      categories: dict[str, list[str]], colors: list) -> list[tuple]:
    """Generate one linear-scale chart per category. Returns list of tuples."""
    out = []
    for cat_name in sorted(categories):
        members = categories[cat_name]
        present = [b for b in members if b in data]
        if not present:
            continue

        fig, ax = _plot_grouped_bars(data, env_names, present,
                                     colors, False,
                                     (max(6, len(present) * 2.5), 4), 7, 0)
        ax.set_ylabel("Operations / second")
        ax.set_title(cat_name)

        slug = cat_name.lower().replace(" ", "_").replace("(", "").replace(")", "")
        path = _chart_path(f"{slug}.png")
        fig.savefig(path, dpi=150)
        plt.close(fig)
        out.append((cat_name, f"benchmark_charts/{slug}.png"))
    return out


def generate_charts(data: dict, env_names: list[str], categories: dict[str, list[str]]):
    """Generate PNG chart files and return a list of (alt_text, rel_path)."""
    if not HAS_MPL:
        return []

    n_envs = len(env_names)
    cmap = plt.colormaps.get("Set2")
    colors = [cmap(i / max(n_envs - 1, 1)) for i in range(n_envs)]

    return _chart_categories(data, env_names, categories, colors)


# ── report generation ─────────────────────────────────────────────────────


def _write_environment_table(w, env_names):
    w("## Environments")
    w()
    w("| # | Platform | Compiler |")
    w("|---|----------|----------|")
    for i, env in enumerate(env_names, 1):
        w(f"| {i} | {env} | gcc / clang |")
    w()


def _write_results_table(w, data, env_names, categories):
    w("## Results")
    w()
    header = "| Category | Benchmark | Iterations | " + " | ".join(env_names) + " |"
    sep = "|---|:---|---:|" + "|".join(":---:" for _ in env_names) + "|"
    w(header)
    w(sep)

    for cat_name in sorted(categories):
        members = sorted(categories[cat_name])
        for bench in members:
            iters = data[bench][env_names[0]][0]
            row = f"| {cat_name} | `{bench}` | {iters:,} |"
            for env in env_names:
                _, _, ops = data[bench][env]
                row += f" {fmt_ops(ops)} |"
            w(row)
    w()


def _write_detail_charts(w, charts):
    if not charts:
        return
    w("## Details by Category")
    w()
    for cat_name, src in charts:
        w(f"### {cat_name}")
        w()
        w(f"![{cat_name}]({src})")
        w()


def generate_report(data: dict, env_names: list[str],
                    charts: list[tuple[str, str]],
                    categories: dict[str, list[str]]) -> str:
    """Produce the BENCHMARK.md content as a string."""
    lines = []
    def w(s=""): lines.append(s)

    w("# LibZenit Benchmarks")
    w()
    w("Automated benchmark results across CI environments. "
      "Generated by `scripts/benchmark_report.py`.")
    w()

    _write_environment_table(w, env_names)
    _write_results_table(w, data, env_names, categories)
    _write_detail_charts(w, charts)

    w("---")
    w()
    w("_Generated from CI benchmark job output._")
    w()

    return "\n".join(lines)


# ── CLI ───────────────────────────────────────────────────────────────────

def parse_args(argv=None):
    p = argparse.ArgumentParser(
        description="Generate LibZenit benchmark report from CI logs."
    )
    p.add_argument(
        "--env", "-e",
        action="append",
        dest="env_names",
        required=True,
        help="Human-readable environment name (e.g. 'macOS Apple Silicon')"
    )
    p.add_argument(
        "--file", "-f",
        action="append",
        dest="file_paths",
        required=True,
        help="Path to the CTest log file for the corresponding --env"
    )
    return p.parse_args(argv)


def main():
    args = parse_args()
    if len(args.env_names) != len(args.file_paths):
        print("error: each --env requires exactly one --file", file=sys.stderr)
        sys.exit(1)

    environments = []
    for env_name, path in zip(args.env_names, args.file_paths):
        text = Path(path).read_text(encoding="utf-8", errors="replace")
        results = parse_log(text)
        if not results:
            print(f"warning: no benchmark lines found in '{path}'", file=sys.stderr)
        environments.append((env_name, results))

    if not environments:
        print("error: no benchmark data parsed", file=sys.stderr)
        sys.exit(1)

    data = build_data(environments)
    env_names = [e[0] for e in environments]
    categories = build_categories(data)

    print(f"Parsed {len(data)} benchmarks across {len(env_names)} environments")
    for cat, members in sorted(categories.items()):
        print(f"  {cat}: {', '.join(members)}")

    charts = generate_charts(data, env_names, categories)
    if charts:
        print(f"Generated {len(charts)} chart(s) in {CHARTS_DIR}/")
    else:
        print("Note: install 'matplotlib' for charts", file=sys.stderr)

    md = generate_report(data, env_names, charts, categories)
    REPORT_PATH.write_text(md, encoding="utf-8")
    print(f"Report written to {REPORT_PATH}")


if __name__ == "__main__":
    main()
