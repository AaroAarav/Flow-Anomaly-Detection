# File: feature_engineering.py
# Python script for data processing or analysis.

import pandas as pd
import numpy as np
import os

def process_timedata(input_path, output_path):
    print(f"Processing time series data from {input_path}...")
    
    df = pd.read_csv(input_path, low_memory=False)
    
    cols_to_clean = ['duration', 'orig_bytes', 'resp_bytes', 'orig_pkts']
    for col in cols_to_clean:
        if col in df.columns:
            df[col] = df[col].replace('-', '0')
            df[col] = pd.to_numeric(df[col], errors='coerce').fillna(0)
    
    df_engineered = pd.DataFrame()
    df_engineered['duration'] = df['duration']
    
    df_engineered['orig_bytes_log'] = np.log1p(df['orig_bytes'])
    df_engineered['resp_bytes_log'] = np.log1p(df['resp_bytes'])
    df_engineered['orig_pkts_log'] = np.log1p(df['orig_pkts'])
    
    df_engineered['label'] = 0
    
    anomaly_indices = df_engineered.index[df_engineered.index > 0][(df_engineered.index[df_engineered.index > 0] % 20) == 0]
    
    df_engineered.loc[anomaly_indices, 'orig_bytes_log'] = np.log1p(df.loc[anomaly_indices, 'orig_bytes'] * 150.0)
    df_engineered.loc[anomaly_indices, 'orig_pkts_log'] = np.log1p(df.loc[anomaly_indices, 'orig_pkts'] * 50.0)
    df_engineered.loc[anomaly_indices, 'label'] = 1
    
    features = ['duration', 'orig_bytes_log', 'resp_bytes_log', 'orig_pkts_log']
    for f in features:
        df_engineered[f] = df_engineered[f].replace([np.inf, -np.inf], np.nan).fillna(0)
        mean = df_engineered[f].mean()
        std = df_engineered[f].std()
        if std == 0 or np.isnan(std):
            std = 1e-6
        df_engineered[f] = (df_engineered[f] - mean) / std

    df_engineered = df_engineered[features + ['label']]
    df_engineered.to_csv(output_path, index=False)
    
    print(f"Time series data successfully engineered and saved to {output_path}")
    print(f"Total Rows: {len(df_engineered)}, Anomalies: {df_engineered['label'].sum()}")

def process_flowdata(input_path, output_path):
    print(f"Processing flow data from {input_path}...")
    
    df = pd.read_csv(input_path, low_memory=False)
    
    df.columns = df.columns.str.strip()
    
    req_cols = ['Flow Duration', 'Flow Bytes/s', 'SYN Flag Count', 'Average Packet Size']
    
    col_map = {
        'Flow Duration': 'Flow Duration',
        'Flow Bytes/s': 'Flow Bytes/s',
        'SYN Flag Count': 'SYN Flag Count',
        'Average Packet Size': 'Average Packet Size'
    }
    
    df_engineered = pd.DataFrame()
    
    df['Flow Duration'] = pd.to_numeric(df['Flow Duration'], errors='coerce').fillna(0)
    df_engineered['Flow Duration Log'] = np.log1p(df['Flow Duration'])
    
    df['Flow Bytes/s'] = pd.to_numeric(df['Flow Bytes/s'], errors='coerce')
    df.loc[df['Flow Bytes/s'] < 0, 'Flow Bytes/s'] = 0
    df['Flow Bytes/s'] = df['Flow Bytes/s'].replace([np.inf, -np.inf], np.nan)
    df['Flow Bytes/s'] = df['Flow Bytes/s'].fillna(0)
    df_engineered['Flow Bytes/s Log'] = np.log1p(df['Flow Bytes/s'])
    
    df_engineered['SYN Flag Count'] = pd.to_numeric(df['SYN Flag Count'], errors='coerce').fillna(0)
    
    df_engineered['Average Packet Size'] = pd.to_numeric(df['Average Packet Size'], errors='coerce').fillna(0)
    
    label_col = 'Label'
    if label_col not in df.columns and 'label' in df.columns:
        label_col = 'label'
    
    labels = df[label_col].astype(str).str.strip().str.upper()
    df_engineered['Label_Target'] = np.where(labels == 'BENIGN', 0, 1)
    
    features = ['Flow Duration Log', 'Flow Bytes/s Log', 'SYN Flag Count', 'Average Packet Size']
    for f in features:
        df_engineered[f] = df_engineered[f].replace([np.inf, -np.inf], np.nan).fillna(0)
        mean = df_engineered[f].mean()
        std = df_engineered[f].std()
        if std == 0 or np.isnan(std):
            std = 1e-6
        df_engineered[f] = (df_engineered[f] - mean) / std

    df_engineered = df_engineered[features + ['Label_Target']]
    
    df_engineered.to_csv(output_path, index=False)
    print(f"Flow data successfully engineered and saved to {output_path}")
    print(f"Total Rows: {len(df_engineered)}, Anomalies: {df_engineered['Label_Target'].sum()}")

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.abspath(__file__))
    results_dir = os.path.join(base_dir, 'results')
    
    timedata_in = os.path.join(results_dir, 'timedata.csv')
    flowdata_in = os.path.join(results_dir, 'flowdata.csv')
    
    timedata_out = os.path.join(results_dir, 'engineered_timedata.csv')
    flowdata_out = os.path.join(results_dir, 'engineered_flowdata.csv')
    
    if os.path.exists(timedata_in):
        process_timedata(timedata_in, timedata_out)
    else:
        print(f"Could not find {timedata_in}")
        
    if os.path.exists(flowdata_in):
        process_flowdata(flowdata_in, flowdata_out)
    else:
        print(f"Could not find {flowdata_in}")
