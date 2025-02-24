import json
import matplotlib.pyplot as plt
import numpy as np

# Load results
file_name = "weak_scaling_results.json"
output_name = "weak_scaling_plot.png"
with open(file_name, "r") as f:
    results = json.load(f)

# Convert string keys to integers
results = {int(k): {int(tk): tv for tk, tv in v.items()} for k, v in results.items()}

# Setup the plot
plt.figure(figsize=(10, 6))
colors = ["blue", "orange", "green", "red"]
markers = ["o-"] * len(results)

# Plot each particles/thread ratio
for ratio, color, marker in zip(sorted(results.keys()), colors, markers):
    threads = sorted(results[ratio].keys())
    times = [results[ratio][t] for t in threads]

    plt.plot(
        threads,
        times,
        marker,
        label=f"#particles/#threads = {ratio}",
        color=color,
        markersize=8,
    )

# Customize the plot
plt.xscale("log", base=2)
plt.yscale("log")
plt.xlabel("Number of Threads")
plt.ylabel("Time (s)")
plt.grid(True, which="both", ls="-", alpha=0.2)
plt.legend(bbox_to_anchor=(1.05, 1), loc="upper left")
plt.title("Weak Scaling")

# Set x-axis ticks to match your thread counts
plt.xticks([1, 2, 4, 8, 16, 32, 64], ["1", "2", "4", "8", "16", "32", "64"])

# Save the plot
plt.savefig(output_name, dpi=300, bbox_inches="tight")
