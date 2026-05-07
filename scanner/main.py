import argparse
import time
import sys
from data_logger import scan_available_wifi_networks, log_scan_results

def parse_command_line_arguments():
    argument_parser = argparse.ArgumentParser(
        description='Wi-Fi Network Scanner for Handover Simulator'
    )
    argument_parser.add_argument(
        '--interval', type=float, default=3.0,
        help='Seconds between scans'
    )
    argument_parser.add_argument(
        '--count', type=int, default=30,
        help='Total number of scans'
    )
    return argument_parser.parse_args()

def run_scanner_main():
    cli_arguments = parse_command_line_arguments()
    total_scans_requested = cli_arguments.count
    scan_interval_seconds = cli_arguments.interval

    print("=" * 60)
    print("  WI-FI NETWORK SCANNER - HANDOVER SIMULATOR")
    print("  Sistemas de Telecomunicações")
    print("=" * 60)

    for current_sequence_id in range(1, total_scans_requested + 1):
        print(f"[{current_sequence_id:03d}/{total_scans_requested}] Scanning...", end=" ", flush=True)
        
        found_networks = scan_available_wifi_networks()

        if not found_networks:
            print("No networks found.")
        else:
            print(f"{len(found_networks)} networks found.")
            log_scan_results(found_networks, current_sequence_id)

        if current_sequence_id < total_scans_requested:
            time.sleep(scan_interval_seconds)

    print("\nScan complete. Data saved in: data/network_log.json")

if __name__ == '__main__':
    try:
        run_scanner_main()
    except KeyboardInterrupt:
        sys.exit(0)
