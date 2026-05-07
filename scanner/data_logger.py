import subprocess
import json
import re
import time
import os
from datetime import datetime

DATA_DIRECTORY = os.path.join(os.path.dirname(__file__), '..', 'data')
LOG_OUTPUT_FILE = os.path.join(DATA_DIRECTORY, 'network_log.json')

def scan_available_wifi_networks() -> list[dict]:
    try:
        scan_result = subprocess.run(
            ['netsh', 'wlan', 'show', 'networks', 'mode=Bssid'],
            capture_output=True,
            text=True,
            encoding='cp850',
            timeout=10
        )
    except FileNotFoundError:
        return []
    except subprocess.TimeoutExpired:
        return []

    discovered_networks = []
    current_network_data = {}

    for output_line in scan_result.stdout.splitlines():
        output_line = output_line.strip()

        ssid_pattern_match = re.match(r'^SSID\s+\d+\s*:\s*(.+)', output_line)
        if ssid_pattern_match:
            if current_network_data:
                discovered_networks.append(current_network_data)
            current_network_data = {'ssid': ssid_pattern_match.group(1).strip()}
            continue

        bssid_pattern_match = re.match(r'^BSSID\s+\d+\s*:\s*([0-9a-fA-F:]+)', output_line)
        if bssid_pattern_match:
            current_network_data['bssid'] = bssid_pattern_match.group(1).strip()
            continue

        signal_pattern_match = re.match(r'^Sinal\s*:\s*(\d+)%|^Signal\s*:\s*(\d+)%', output_line)
        if signal_pattern_match:
            signal_percentage = int(signal_pattern_match.group(1) or signal_pattern_match.group(2))
            rssi_dbm_value = (signal_percentage / 2) - 100
            current_network_data['rssi_dbm'] = round(rssi_dbm_value, 1)
            current_network_data['signal_pct'] = signal_percentage
            continue

    if current_network_data:
        discovered_networks.append(current_network_data)

    return [network for network in discovered_networks if 'bssid' in network and 'rssi_dbm' in network]

def log_scan_results(networks_list: list[dict], sequence_id: int) -> None:
    os.makedirs(DATA_DIRECTORY, exist_ok=True)

    log_history = []
    if os.path.exists(LOG_OUTPUT_FILE):
        with open(LOG_OUTPUT_FILE, 'r', encoding='utf-8') as file_handle:
            try:
                log_history = json.load(file_handle)
            except json.JSONDecodeError:
                log_history = []

    new_scan_record = {
        'scan_id': sequence_id,
        'timestamp': datetime.now().isoformat(),
        'networks': networks_list
    }
    log_history.append(new_scan_record)

    with open(LOG_OUTPUT_FILE, 'w', encoding='utf-8') as file_handle:
        json.dump(log_history, file_handle, indent=2, ensure_ascii=False)
