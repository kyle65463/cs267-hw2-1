import subprocess
import json

# Configuration
particle_counts = [1000, 10000, 100000, 1000000]
thread_counts = [1, 2, 4, 8, 16, 32, 64]
seed = 99
repeats = 3  # Run multiple times to get average
output_name = "strong_scaling_results.json"

def save_result(n, t, avg_time):
    try:
        # Load existing results
        with open(output_name, "r") as f:
            results = json.load(f)
    except FileNotFoundError:
        results = {}
    
    # Update results
    if n not in results:
        results[n] = {}
    results[n][t] = avg_time
    
    # Save updated results
    with open(output_name, "w") as f:
        json.dump(results, f, indent=4)

print("Running serial version...")
for n in particle_counts:
    print(f"\nParticle count: {n}")
    times = []
    r = 1 if n == 1000000 else repeats
    for i in range(r):
        print(f"  Run {i+1}/{r} with n={n}")
        cmd = f"./serial -n {n} -s {seed} -o out/serial-n{n}.out"
        output = subprocess.check_output(cmd, shell=True, universal_newlines=True)
        time = float(output.split("=")[1].split()[0])
        times.append(time)
        print(f"    Time: {time:.4f} seconds")

    avg_time = sum(times) / len(times)
    print(f"  Average time: {avg_time:.4f} seconds")
    save_result(n, 1, avg_time)  # Save after each serial measurement

print("Running OpenMP version...")
for n in particle_counts:
    print(f"\nParticle count: {n}")
    for t in thread_counts[1:]:  # Skip 1 as we used serial for that
        print(f"  Thread count: {t}")
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
        save_result(n, t, avg_time)  # Save after each OpenMP measurement

print("Data collection complete. Results saved to timing_results.json")
