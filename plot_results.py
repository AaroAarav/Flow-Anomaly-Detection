import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

def plot_best_f1(csv_path, dataset_name, output_filename):
    if not os.path.exists(csv_path):
        print(f"File not found: {csv_path}")
        return
        
    df = pd.read_csv(csv_path)
    # Find the row with the maximum F1_Score for each Algorithm
    best_df = df.loc[df.groupby('Algorithm')['F1_Score'].idxmax()]
    
    # Sort for better visualization
    best_df = best_df.sort_values('F1_Score', ascending=False)
    
    plt.figure(figsize=(10, 6))
    sns.barplot(x='F1_Score', y='Algorithm', data=best_df, palette='viridis')
    plt.title(f'Peak F1 Score by Algorithm - {dataset_name}', fontsize=14, fontweight='bold')
    plt.xlabel('Maximum F1 Score', fontsize=12)
    plt.ylabel('Algorithm', fontsize=12)
    plt.xlim(0, 1.05)
    plt.grid(axis='x', linestyle='--', alpha=0.7)
    
    # Add labels to bars
    for index, value in enumerate(best_df['F1_Score']):
        plt.text(value + 0.01, index, f"{value:.4f}", va='center')
        
    plt.tight_layout()
    plt.savefig(output_filename, dpi=300)
    plt.close()

def plot_metrics(csv_path, output_latency, output_ram):
    if not os.path.exists(csv_path):
        return
        
    df = pd.read_csv(csv_path)
    # Group by algorithm and take mean latency and max RAM
    metrics_df = df.groupby('Algorithm').agg({
        'Latency_ms': 'mean',
        'Peak_RAM_KB': 'max'
    }).reset_index()
    
    metrics_df = metrics_df.sort_values('Latency_ms')
    
    # Latency Plot
    plt.figure(figsize=(10, 6))
    sns.barplot(x='Latency_ms', y='Algorithm', data=metrics_df, palette='rocket')
    plt.title('Average Execution Latency (ms) by Algorithm', fontsize=14, fontweight='bold')
    plt.xlabel('Latency (milliseconds)', fontsize=12)
    plt.xscale('log') # Log scale because ABOD is 140ms and others are 0.1ms
    plt.ylabel('')
    plt.grid(axis='x', linestyle='--', alpha=0.7)
    
    for index, row in metrics_df.iterrows():
        plt.text(row['Latency_ms'] * 1.1, index, f"{row['Latency_ms']:.2f} ms", va='center')
        
    plt.tight_layout()
    plt.savefig(output_latency, dpi=300)
    plt.close()
    
    # RAM Plot
    metrics_df = metrics_df.sort_values('Peak_RAM_KB')
    plt.figure(figsize=(10, 6))
    sns.barplot(x='Peak_RAM_KB', y='Algorithm', data=metrics_df, palette='mako')
    plt.title('Peak RAM Usage (KB) by Algorithm', fontsize=14, fontweight='bold')
    plt.xlabel('Peak RAM (Kilobytes)', fontsize=12)
    plt.ylabel('')
    plt.grid(axis='x', linestyle='--', alpha=0.7)
    
    for index, row in metrics_df.iterrows():
        plt.text(row['Peak_RAM_KB'] + 5, index, f"{int(row['Peak_RAM_KB'])} KB", va='center')
        
    plt.tight_layout()
    plt.savefig(output_ram, dpi=300)
    plt.close()

if __name__ == "__main__":
    ts_csv = "results/ts_param_sweep.csv"
    flow_csv = "results/flow_param_sweep.csv"
    
    print("Generating F1 Score plots...")
    plot_best_f1(ts_csv, "Time Series Dataset", "results/plot_f1_ts.png")
    plot_best_f1(flow_csv, "Flow Specific Dataset", "results/plot_f1_flow.png")
    
    print("Generating Metric plots...")
    # Flow and TS will have similar latency/RAM profiles, just plot Flow for metrics
    plot_metrics(flow_csv, "results/plot_latency.png", "results/plot_ram.png")
    
    print("Plots generated successfully in the results/ folder.")
