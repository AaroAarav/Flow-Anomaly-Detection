# Anomaly Edge Detection

This project provides an edge-optimized framework for detecting anomalies in network flow data and general time-series data. The core anomaly detection algorithms are implemented in C to minimize latency and memory consumption, making them highly suitable for edge devices. 

Python is used for data preprocessing and result visualization.

## Project Structure

- `feature_engineering.py`: Python script for preprocessing raw datasets (extracting features, log scaling, standardization, and injecting synthetic anomalies).
- `plot_results.py`: Python script for generating comparison plots (F1-score, latency, and peak RAM usage) across various algorithms.
- `flow_specific/`: C implementations and `Makefile` for anomaly detection algorithms tailored to network flow data.
- `time_series/`: C implementations and `Makefile` for anomaly detection algorithms tailored to time-series data.
- `analysis/`: Python scripts for hyperparameter analysis and algorithm comparison.
- `results/`: Contains raw data (`timedata.csv`, `flowdata.csv`), engineered datasets, metrics CSVs, and generated plots.
- `plots/`: Contains granular algorithm sweep plots.
- `logs/`: Directory for execution logs.

## Supported Algorithms

This framework implements several edge-optimized anomaly detection methods:

- **Angle-Based Outlier Detection (ABOD)**: Evaluates the variance of angles between a target point and all other pairs of points. Outliers reside at the edge of the data distribution, resulting in a significantly lower variance of angles compared to normal points.
- **Local Outlier Factor (LOF)**: Compares the local density of a given point to the local densities of its k-nearest neighbors. Points with a substantially lower density than their neighbors are flagged as anomalies.
- **Epsilon Density (EpsDensity)**: Calculates the proportion of training points that fall within a fixed radius (epsilon) of a target point. If a point has very few neighbors within this radius, it is considered an anomaly.
- **Euclidean Distance**: Computes the straight-line (L2) distance from a data point to the centroid of the normal training data. Points that exceed a certain distance threshold are marked as anomalous.
- **Manhattan Distance**: Computes the sum of absolute differences (L1 distance) between a point and the normal data centroid across all feature dimensions. It is computationally cheaper and often more robust in high-dimensional spaces than Euclidean distance.
- **Mahalanobis Distance**: Measures distance from the centroid while accounting for the covariance between features. It is highly effective for multivariate data because it correctly scales the distance according to the shape of the data distribution.
- **Hypercube**: Constructs a bounding box (minimum and maximum bounds per feature) around the normal training data. Points falling outside these boundaries are penalized based on their distance from the bounds.
- **Streaming Isolation Forest**: An ensemble method that isolates anomalies by randomly partitioning the data via decision trees. Anomalies are easier to isolate and thus have shorter average path lengths in the trees. The streaming variant is optimized for bounded memory usage.
- **Holt-Winters**: A time-series forecasting technique that models the level, trend, and seasonality of the data. Anomalies are detected when the actual observed value deviates significantly from the algorithm's forecasted value.

## How to Run

If you have just cloned the repository, follow these steps to execute the algorithms and view the results.

### 1. Set Up Python Environment
First, create a virtual environment and install the required dependencies for preprocessing and plotting.
```powershell
python -m venv venv
.\venv\Scripts\Activate.ps1
pip install pandas matplotlib seaborn
```

### 2. Feature Engineering
Process the raw data into standardized formats for the C algorithms:
```powershell
python feature_engineering.py
```

### 3. Compile and Run Flow Data Algorithms
Navigate to the `flow_specific` directory, compile the C code using `make` (or `mingw32-make` on Windows), and run the executable.
```powershell
cd flow_specific
# If 'make' is not available, use 'mingw32-make' instead
mingw32-make
.\bin_flow_anomaly.exe -d ..\results\engineered_flowdata.csv
cd ..
```

### 4. Compile and Run Time-Series Algorithms
Navigate to the `time_series` directory, compile, and run:
```powershell
cd time_series
mingw32-make
.\bin_ts_anomaly.exe -d ..\results\engineered_timedata.csv
cd ..
```

### 5. Visualize Results
Once the algorithms finish, generate the visual performance comparisons (F1 Score, Latency, RAM usage) in the `results/` folder:
```powershell
python plot_results.py
```
