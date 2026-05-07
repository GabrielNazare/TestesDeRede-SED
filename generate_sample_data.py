import os

script_directory = os.path.dirname(os.path.abspath(__file__))
output_directory = os.path.join(script_directory, 'output')
os.makedirs(output_directory, exist_ok=True)

with open(os.path.join(output_directory, 'simulation_metrics.csv'), 'w') as csvFile:
    csvFile.write("metric,value\n")
    csvFile.write("total_handovers,4\n")
    csvFile.write("ping_pong_rate_pct,0.0\n")
    csvFile.write("average_rsrp_dbm,-72.5\n")
    csvFile.write("coverage_pct,95.0\n")
    csvFile.write("simulation_duration_s,40.0\n")

with open(os.path.join(output_directory, 'handover_events.csv'), 'w') as csvFile:
    csvFile.write("timestamp_s,from_station,to_station,rsrp_dbm\n")
    csvFile.write("10.5,eNB-A,eNB-B,-65.2\n")
    csvFile.write("22.1,eNB-B,eNB-C,-62.1\n")
    csvFile.write("31.8,eNB-C,eNB-B,-68.4\n")
    csvFile.write("38.2,eNB-B,eNB-A,-66.0\n")

with open(os.path.join(output_directory, 'position_trace.csv'), 'w') as csvFile:
    csvFile.write("timestamp_s,x,y\n")
    for timestep in range(41):
        csvFile.write(f"{timestep}.0,{10 + timestep * 5},0.0\n")

with open(os.path.join(output_directory, 'signal_trace.csv'), 'w') as csvFile:
    csvFile.write("timestamp_s,station_id,rsrp_dbm\n")
    for timestep in range(41):
        rsrpStationA = -50 - (timestep * 1.5)
        rsrpStationB = -90 + (timestep * 1.5) if timestep < 20 else -60 - ((timestep - 20) * 1.5)
        csvFile.write(f"{timestep}.0,eNB-A,{rsrpStationA:.1f}\n")
        csvFile.write(f"{timestep}.0,eNB-B,{rsrpStationB:.1f}\n")

print(f"Sample CSV data created in: {output_directory}/")
