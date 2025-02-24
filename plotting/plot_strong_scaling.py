import json
import matplotlib.pyplot as plt
import numpy as np

# Load results
file_name = "strong_scaling_results.json"
output_name = "strong_scaling_plot.png"
with open(file_name, "r") as f:
    results = json.load(f)

# Convert string keys to integers
results = {int(k): {int(tk): tv for tk, tv in v.items()} for k, v in results.items()}

# Setup the plot
plt.figure(figsize=(10, 6))
colors = ["blue", "orange", "green", "red"]
markers = ["o-"] * len(results)

# Plot each particle count
for n, color, marker in zip(sorted(results.keys()), colors, markers):
    threads = sorted(results[n].keys())
    times = [results[n][t] for t in threads]

    plt.plot(
        threads, times, marker, label=f"#particles = {n}", color=color, markersize=8
    )

# Add reference line with slope -1
x_ref = np.array([1, 64], dtype=float)
y_ref = 100 * x_ref ** (-1)
plt.plot(x_ref, y_ref, "k--", label="Slope = -1")

# Customize the plot
plt.xscale("log", base=2)
plt.yscale("log")
plt.xlabel("Number of Threads")
plt.ylabel("Time (s)")
plt.grid(True, which="both", ls="-", alpha=0.2)
plt.legend(bbox_to_anchor=(1.05, 1), loc="upper left")
plt.title("Strong Scaling")

# Set x-axis ticks to match your thread counts
plt.xticks([1, 2, 4, 8, 16, 32, 64], ["1", "2", "4", "8", "16", "32", "64"])

# Save the plot
plt.savefig(output_name, dpi=300, bbox_inches="tight")
