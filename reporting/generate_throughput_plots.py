import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import glob

results_dir = "results/throughput"
plots_dir = "plots/throughput_all_queues"
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
    data["Average Throughput per Thread (jobs/sec/thread)"] = pd.to_numeric(data["Average Throughput per Thread (jobs/sec/thread)"], errors="coerce")
    data = data.dropna(subset=["Producer/Consumer Count", "Average Throughput per Thread (jobs/sec/thread)", "Queue"])

    if data.empty:
        print(f"Skipping {file_path} due to no valid data.")
        continue

    # Create a new figure for the plot
    plt.figure()

    job_count_str = os.path.basename(file_path).replace("throughput_job_count_", "").replace(".csv", "")

    # Plot data for each queue type
    for queue_type in data["Queue"].unique():
        subset = data[data["Queue"] == queue_type].sort_values("Producer/Consumer Count")

        # Plot original data
        line = plt.plot(subset["Producer/Consumer Count"], subset["Average Throughput per Thread (jobs/sec/thread)"], marker="o", linestyle="-", label=queue_type)

        # Calculate and plot linear trendline
        # if len(subset["Producer/Consumer Count"]) > 1:
        #     coeffs = np.polyfit(subset["Producer/Consumer Count"], subset["Average Throughput per Thread (jobs/sec/thread)"], 1)
        #     trendline = np.poly1d(coeffs)
        #     plt.plot(subset["Producer/Consumer Count"], trendline(subset["Producer/Consumer Count"]), linestyle="--", color=line[0].get_color())

    # Configure and save the plot
    plt.title(f"Throughput per Thread vs. P/C Count ({job_count_str} Jobs)")
    plt.xlabel("Producer/Consumer Count")
    plt.ylabel("Average Throughput per Thread (jobs/sec/thread)")
    plt.legend()
    plt.grid(True, which="both", ls=":")
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
    plt.tight_layout()

    filename = os.path.join(plots_dir, f"producer_consumer_count_vs_throughput_per_thread_{job_count_str}.png")
    plt.savefig(filename)
    print(f"Saved plot to {filename}")

    # Show and close the figure
    # plt.show() # Commented out to not block for each plot
    plt.close()

    # Generate a second plot excluding std::queue (Blocking)
    data_no_std = data[data["Queue"] != "std::queue (Blocking)"]

    if data_no_std.empty or len(data_no_std["Queue"].unique()) == 0:
        print(f"Skipping second plot for {file_path} as no other queues are present.")
        continue

    plt.figure()

    for queue_type in data_no_std["Queue"].unique():
        subset = data_no_std[data_no_std["Queue"] == queue_type].sort_values("Producer/Consumer Count")

        # Plot original data
        line = plt.plot(subset["Producer/Consumer Count"], subset["Average Throughput per Thread (jobs/sec/thread)"], marker="o", linestyle="-", label=queue_type)

        # Calculate and plot linear trendline
        # if len(subset["Producer/Consumer Count"]) > 1:
        #     coeffs = np.polyfit(subset["Producer/Consumer Count"], subset["Average Throughput per Thread (jobs/sec/thread)"], 1)
        #     trendline = np.poly1d(coeffs)
        #     plt.plot(subset["Producer/Consumer Count"], trendline(subset["Producer/Consumer Count"]), linestyle="--", color=line[0].get_color())

    # Configure and save the plot
    plt.title(f"Throughput per Thread vs. P/C Count ({job_count_str} Jobs)")
    plt.xlabel("Producer/Consumer Count")
    plt.ylabel("Average Throughput per Thread (jobs/sec/thread)")
    plt.legend()
    plt.grid(True, which="both", ls=":")
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
    plt.tight_layout()

    filename_no_std = os.path.join(plots_dir, f"producer_consumer_count_vs_throughput_per_thread_{job_count_str}_no_std_queue.png")
    plt.savefig(filename_no_std)
    print(f"Saved plot to {filename_no_std}")

    # Show and close the figure
    # plt.show() # Commented out to not block for each plot
    plt.close()
