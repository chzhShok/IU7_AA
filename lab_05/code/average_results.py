import csv
from collections import defaultdict
from pathlib import Path

INPUT = Path("../result/experiments_time.csv")
OUTPUT = Path("../result/experiments_time_avg.csv")

# (vertices, threads) -> [time_us, ...]
groups = defaultdict(list)

with INPUT.open("r", newline="", encoding="utf-8") as f:
    reader = csv.DictReader(f)
    for row in reader:
        if not row["vertices"]:
            continue
        t = int(row["threads"])
        N = int(row["N"])
        time_us = int(row["time_us"])
        groups[(t, N)].append(time_us)

with OUTPUT.open("w", newline="", encoding="utf-8") as f:
    fieldnames = ["threads", "N", "avg_time_us", "avg_time_ms", "count"]
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()

    for (t, N), times in sorted(groups.items()):
        avg_us = sum(times) / len(times)
        avg_ms = avg_us / 1000.0
        writer.writerow({
            "threads": t,
            "N": N,
            "avg_time_us": f"{avg_us:.0f}",
            "avg_time_ms": f"{avg_ms:.3f}",
            "count": len(times),
        })

print(f"Saved averages to {OUTPUT}")
