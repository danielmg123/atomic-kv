#!/usr/bin/env python3
import glob
import pandas as pd
import matplotlib.pyplot as plt
import os
import re

# 1) Gather only our benchmark CSVs
paths = sorted(glob.glob("benchmarks/results/bench_*.csv"))
if not paths:
    print("No CSV files found in benchmarks/results/")
    exit(1)

# 2) Load & concatenate
dfs = []
for p in paths:
    df = pd.read_csv(p)
    df["source_file"] = os.path.basename(p)
    dfs.append(df)
df = pd.concat(dfs, ignore_index=True)

# 2.1) Keep only the “_mean” rows so we don’t mix in medians/stddevs
df = df[df["name"].str.endswith("_mean")].copy()

# 2.2) Extract a numeric `threads` column from the name (default → 1)
def extract_threads(s):
    m = re.search(r"threads:(\d+)", s)
    return int(m.group(1)) if m else 1
df["threads"] = df["name"].apply(extract_threads)

# 2.3) Clean up the benchmark name (drop “/threads:X” and “_mean”)
df["benchmark"] = (
    df["name"]
      .str.replace(r"/threads:\d+", "", regex=True)
      .str.replace(r"_mean$", "", regex=True)
)

# 3) Convert real_time from ns → seconds
if "time_unit" in df.columns and df.loc[0, "time_unit"] == "ns":
    df["real_time_s"] = df["real_time"] / 1e9
else:
    df["real_time_s"] = df["real_time"]

# 4) Pivot: average time per (benchmark, threads)
pivot = (
    df.groupby(["benchmark", "threads"])["real_time_s"]
      .mean()
      .reset_index()
      .pivot(index="threads", columns="benchmark", values="real_time_s")
)

# 5) Plot each benchmark’s scaling curve
outdir = "benchmarks/results/plots"
os.makedirs(outdir, exist_ok=True)

for name in pivot.columns:
    plt.figure()
    pivot[name].plot(marker="o")
    plt.title(name)
    plt.xlabel("Threads")
    plt.ylabel("Avg time (s)")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"{outdir}/{name.replace('/', '_')}_scaling.png")
    plt.close()

# 6) Combined overview
plt.figure()
for name in pivot.columns:
    plt.plot(pivot.index, pivot[name], marker="o", label=name)
plt.title("Scaling behavior")
plt.xlabel("Threads")
plt.ylabel("Avg time (s)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(f"{outdir}/overview_scaling.png")
plt.show()