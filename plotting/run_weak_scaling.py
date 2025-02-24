import subprocess
import json

# Configuration
particles_per_thread = [1000, 2000, 5000, 10000]
thread_counts = [1, 2, 4, 8, 16, 32, 64]
seed = 100
repeats = 2  # Run multiple times to get average
output_name = "weak_scaling_results.json"


def save_result(ratio, t, avg_time):
    try:
        # Load existing results
        with open(output_name, "r") as f:
            results = json.load(f)
    except FileNotFoundError:
        results = {}

    # Update results
    if ratio not in results:
        results[ratio] = {}
    results[ratio][t] = avg_time

    # Save updated results
    with open(output_name, "w") as f:
        json.dump(results, f, indent=4)


# Run serial version
print("\nRunning serial version for single thread cases...")
for ratio in particles_per_thread:
    n = ratio * 1  # Total particles for single thread
    print(f"\nParticles per thread: {ratio} (total particles: {n})")
    times = []

    for i in range(repeats):
        print(f"  Run {i+1}/{repeats} with n={n}")
        cmd = f"./serial -n {n} -s {seed} -o out/serial-n{n}.out"
        output = subprocess.check_output(cmd, shell=True, universal_newlines=True)
        time = float(output.split("=")[1].split()[0])
        times.append(time)
        print(f"    Time: {time:.4f} seconds")

    avg_time = sum(times) / len(times)
    print(f"  Average time: {avg_time:.4f} seconds")
    save_result(ratio, 1, avg_time)  # Save single thread results

# Run OpenMP version
print("\nRunning OpenMP version...")
for ratio in particles_per_thread:
    print(f"\nParticles per thread: {ratio}")

    for t in thread_counts[1:]:  # Skip 1 as we used serial for that
        n = ratio * t  # Total particles scales with thread count
        print(f"  Thread count: {t} (total particles: {n})")
        times = []

        for i in range(repeats):
            print(f"    Run {i+1}/{repeats} with n={n}, t={t}")
            cmd = f"OMP_NUM_THREADS={t} ./openmp -n {n} -s {seed} -o out/openmp-n{n}-t{t}.out"
            output = subprocess.check_output(cmd, shell=True, universal_newlines=True)
            time = float(output.split("=")[1].split()[0])
            times.append(time)
            print(f"      Time: {time:.4f} seconds")

        avg_time = sum(times) / len(times)
        print(f"    Average time: {avg_time:.4f} seconds")
        save_result(ratio, t, avg_time)  # Save after each measurement

print(f"Data collection complete. Results saved to {output_name}")
