# File: compare_algorithms.py
# Python script for data processing or analysis.

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

def plot_best_f1_comparison(csv_path, dataset_name):
    if not os.path.exists(csv_path):
        return

    df = pd.read_csv(csv_path)
    
    best_results = df.loc[df.groupby('Algorithm')['F1_Score'].idxmax()]
    
    plt.figure(figsize=(8, 6))
    sns.set_theme(style="whitegrid")
    
    ax = sns.barplot(data=best_results, x='Algorithm', y='F1_Score', hue='Algorithm', palette='viridis', legend=False)
    
    plt.title(f'Peak F1-Score Comparison ({dataset_name})', fontsize=14, pad=15)
    plt.xlabel('Algorithm', fontsize=12)
    plt.ylabel('Best F1-Score', fontsize=12)
    plt.ylim(0, 1.1)
    
    for p in ax.patches:
        ax.annotate(format(p.get_height(), '.4f'), 
                    (p.get_x() + p.get_width() / 2., p.get_height()), 
                    ha = 'center', va = 'center', 
                    xytext = (0, 9), 
                    textcoords = 'offset points')

    output_filename = f'../plots/{dataset_name.lower()}_best_f1_comparison.png'
    plt.tight_layout()
    plt.savefig(output_filename, dpi=300)
    plt.close()
    
    print(f"[SUCCESS] Saved comparison plot to {output_filename}")

if __name__ == "__main__":
    os.makedirs('../plots', exist_ok=True)
    
    print("--- Generating Algorithm Comparison Plots ---")
    plot_best_f1_comparison('../results/ts_param_sweep.csv', 'Time_Series')
    plot_best_f1_comparison('../results/flow_param_sweep.csv', 'Flow_Specific')