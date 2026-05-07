
import argparse
import os
import sys
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np


def load_csv_safe(filepath: str) -> pd.DataFrame | None:
    if not os.path.exists(filepath):
        print(f"[WARN] File not found: {filepath}")
        return None
    df = pd.read_csv(filepath)
    if df.empty:
        print(f"[WARN] Empty file: {filepath}")
        return None
    return df


def plot_rsrp_vs_time(signal_df: pd.DataFrame, handover_df: pd.DataFrame | None, output_dir: str):
    fig, ax = plt.subplots(figsize=(14, 6))

    stations = signal_df['station_id'].unique()
    colors = plt.cm.Set2(np.linspace(0, 1, len(stations)))

    for station, color in zip(stations, colors):
        mask = signal_df['station_id'] == station
        ax.plot(signal_df.loc[mask, 'timestamp_s'],
                signal_df.loc[mask, 'rsrp_dbm'],
                label=station, color=color, alpha=0.8, linewidth=1.5)

    if handover_df is not None and not handover_df.empty:
        for _, ho in handover_df.iterrows():
            ax.axvline(x=ho['timestamp_s'], color='red', linestyle='--', alpha=0.6, linewidth=1)
            ax.annotate(f"HO → {ho['to_station']}",
                        xy=(ho['timestamp_s'], ho['rsrp_dbm']),
                        fontsize=7, color='red', rotation=45,
                        xytext=(5, 10), textcoords='offset points')

    ax.axhline(y=-110, color='gray', linestyle=':', alpha=0.5, label='Limiar cobertura (-110 dBm)')
    ax.set_xlabel('Tempo (s)', fontsize=12)
    ax.set_ylabel('RSRP (dBm)', fontsize=12)
    ax.set_title('📡 RSRP vs Tempo — Monitoramento de Sinal por Estação', fontsize=14, fontweight='bold')
    ax.legend(loc='lower left', fontsize=9)
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(output_dir, 'rsrp_vs_time.png'), dpi=150)
    plt.close(fig)
    print("[OK] rsrp_vs_time.png")


def plot_position_map(position_df: pd.DataFrame, handover_df: pd.DataFrame | None, output_dir: str):
    fig, ax = plt.subplots(figsize=(14, 5))

    ax.plot(position_df['x'], position_df['y'], 'b-o', markersize=4, alpha=0.6, label='Trajetória UE')

    base_stations = [("eNB-A", 0, 0), ("eNB-B", 100, 0), ("eNB-C", 200, 0)]
    for name, bx, by in base_stations:
        ax.plot(bx, by, 's', markersize=14, color='green', zorder=5)
        ax.annotate(name, (bx, by), fontsize=10, fontweight='bold',
                    xytext=(0, 15), textcoords='offset points', ha='center')

    if handover_df is not None and not handover_df.empty:
        for _, ho in handover_df.iterrows():
            closest = position_df.iloc[(position_df['timestamp_s'] - ho['timestamp_s']).abs().argsort()[:1]]
            if not closest.empty:
                ax.plot(closest['x'].values[0], closest['y'].values[0],
                        'r*', markersize=18, zorder=10, label='_nolegend_')

    ho_patch = plt.Line2D([0], [0], marker='*', color='red', linestyle='None', markersize=12, label='Handover')
    bs_patch = plt.Line2D([0], [0], marker='s', color='green', linestyle='None', markersize=10, label='Base Station')
    ax.legend(handles=[ax.get_lines()[0], bs_patch, ho_patch], loc='upper right')

    ax.set_xlabel('Posição X (metros)', fontsize=12)
    ax.set_ylabel('Posição Y (metros)', fontsize=12)
    ax.set_title('🗺️ Mapa de Posição — Trajetória do UE e Handovers', fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(output_dir, 'position_map.png'), dpi=150)
    plt.close(fig)
    print("[OK] position_map.png")


def plot_kpi_summary(metrics_df: pd.DataFrame, output_dir: str):
    fig, ax = plt.subplots(figsize=(10, 5))

    metrics = dict(zip(metrics_df['metric'], metrics_df['value']))

    kpi_names = ['Total Handovers', 'Ping-Pong Rate (%)', 'Avg RSRP (dBm)', 'Coverage (%)']
    kpi_values = [
        metrics.get('total_handovers', 0),
        metrics.get('ping_pong_rate_pct', 0),
        abs(metrics.get('average_rsrp_dbm', 0)),
        metrics.get('coverage_pct', 0),
    ]
    colors = ['#3498db', '#e74c3c', '#f39c12', '#2ecc71']

    bars = ax.barh(kpi_names, kpi_values, color=colors, edgecolor='white', height=0.5)

    for bar, val in zip(bars, kpi_values):
        ax.text(bar.get_width() + 0.5, bar.get_y() + bar.get_height() / 2,
                f'{val:.1f}', va='center', fontsize=11, fontweight='bold')

    ax.set_xlabel('Valor', fontsize=12)
    ax.set_title('📊 KPIs de Telecom — Resumo da Simulação', fontsize=14, fontweight='bold')
    ax.grid(axis='x', alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(output_dir, 'kpi_summary.png'), dpi=150)
    plt.close(fig)
    print("[OK] kpi_summary.png")


def main():
    parser = argparse.ArgumentParser(description='Handover Simulation Analysis Dashboard')
    parser.add_argument('--input-dir', default='output', help='Directory with CSV files from simulator')
    parser.add_argument('--output-dir', default='analysis/figures', help='Directory for output figures')
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)
    plt.style.use('seaborn-v0_8-darkgrid')

    print("=" * 50)
    print("  HANDOVER ANALYSIS DASHBOARD")
    print("=" * 50)

    signal_df   = load_csv_safe(os.path.join(args.input_dir, 'signal_trace.csv'))
    handover_df = load_csv_safe(os.path.join(args.input_dir, 'handover_events.csv'))
    position_df = load_csv_safe(os.path.join(args.input_dir, 'position_trace.csv'))
    metrics_df  = load_csv_safe(os.path.join(args.input_dir, 'simulation_metrics.csv'))

    if signal_df is not None:
        plot_rsrp_vs_time(signal_df, handover_df, args.output_dir)

    if position_df is not None:
        plot_position_map(position_df, handover_df, args.output_dir)

    if metrics_df is not None:
        plot_kpi_summary(metrics_df, args.output_dir)

    print(f"\nAll figures saved to: {args.output_dir}/")


if __name__ == '__main__':
    main()
