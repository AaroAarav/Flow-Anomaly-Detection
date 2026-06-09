# File: hyperparameter_analysis.py
# Python script for data processing or analysis.

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

def plot_parameter_sweep(csv_path, dataset_name):
    if not os.path.exists(csv_path):
        print(f"[ERROR] Could not find {csv_path}. Run the C sweep first.")
        return

    df = pd.read_csv(csv_path)
    
    sns.set_theme(style="whitegrid")
    
    algorithms = df['Algorithm'].unique()
    
    for algo in algorithms:
        algo_data = df[df['Algorithm'] == algo]
        
        plt.figure(figsize=(10, 6))
        
        plt.plot(algo_data['Threshold'], algo_data['F1_Score'], marker='o', label='F1-Score', color='b', linewidth=2)
        plt.plot(algo_data['Threshold'], algo_data['Precision'], marker='s', label='Precision', color='g', linestyle='--')
        plt.plot(algo_data['Threshold'], algo_data['Recall'], marker='^', label='Recall', color='r', linestyle='-.')
        
        plt.title(f'{dataset_name}: {algo} Hyperparameter Sweep', fontsize=14, pad=15)
        plt.xlabel('Threshold Value', fontsize=12)
        plt.ylabel('Metric Score (0.0 - 1.0)', fontsize=12)
        plt.ylim(-0.05, 1.05)
        plt.legend(loc='lower right')
        
        safe_algo_name = algo.replace(" ", "_").lower()
        output_filename = f'../plots/{dataset_name.lower()}_{safe_algo_name}_sweep.png'
        plt.tight_layout()
        plt.savefig(output_filename, dpi=300)
        plt.close()
        
        print(f"[SUCCESS] Saved sweep plot to {output_filename}")

if __name__ == "__main__":
    os.makedirs('../plots', exist_ok=True)
    
    print("--- Generating Hyperparameter Sweep Plots ---")
    plot_parameter_sweep('../results/ts_param_sweep.csv', 'Time_Series')
    plot_parameter_sweep('../results/flow_param_sweep.csv', 'Flow_Specific')