import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import glob

results_dir = "results/latency"
plots_dir = "plots/latency"
os.makedirs(plots_dir, exist_ok=True)

csv_files = glob.glob(os.path.join(results_dir, "*.csv"))

if not csv_files:
    print(f"No CSV files found in {results_dir}")
    exit()

for file_path in csv_files:
    print(f"Processing {file_path}...")
    # Read and process the data
    try:
        data = pd.read_csv(file_path)
    except FileNotFoundError:
        print(f"Error: The file {file_path} was not found.")
        continue

    data = data.loc[:, ~data.columns.str.contains("^Unnamed")]
    data["Producer/Consumer Count"] = pd.to_numeric(data["Producer/Consumer Count"], errors="coerce")
    data["Average Latency (ns)"] = pd.to_numeric(data["Average Latency (ns)"], errors="coerce")
    data = data.dropna(subset=["Producer/Consumer Count", "Average Latency (ns)", "Queue"])

    if data.empty:
        print(f"Skipping {file_path} due to no valid data.")
        continue

    # Create a new figure for the plot
    plt.figure()

    job_count_str = os.path.basename(file_path).replace("latency_job_count_", "").replace(".csv", "")

    # Plot data for each queue type
    for queue_type in data["Queue"].unique():
        subset = data[data["Queue"] == queue_type].sort_values("Producer/Consumer Count")

        # Plot original data
        line = plt.plot(subset["Producer/Consumer Count"], subset["Average Latency (ns)"], marker="o", linestyle="-", label=queue_type)

        # Calculate and plot linear trendline
        # if len(subset["Producer/Consumer Count"]) > 1:
        #     coeffs = np.polyfit(subset["Producer/Consumer Count"], subset["Average Latency (ns)"], 1)
        #     trendline = np.poly1d(coeffs)
        #     plt.plot(subset["Producer/Consumer Count"], trendline(subset["Producer/Consumer Count"]), linestyle="--", color=line[0].get_color())

    # Configure and save the plot
    plt.title(f"Average Latency vs. P/C Count ({job_count_str} Jobs)")
    plt.xlabel("Producer/Consumer Count")
    plt.ylabel("Average Latency (ns)")
    plt.legend()
    plt.grid(True, which="both", ls=":")
    plt.tight_layout()

    filename = os.path.join(plots_dir, f"producer_consumer_count_vs_latency_{job_count_str}.png")
    plt.savefig(filename)
    print(f"Saved plot to {filename}")

    # Show and close the figure
    # plt.show() # Commented out to not block for each plot
    plt.close()

