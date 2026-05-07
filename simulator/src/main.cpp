#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include "SimulationEngine.h"
#include "MetricsCollector.h"
#include "BaseStation.h"
#include "MobileNode.h"

struct SimulationConfig {
    std::string mode          = "synthetic";
    std::string jsonFilePath  = "../data/network_log.json";
    double      tttMs         = 160.0;
    double      hysteresisDb  = 3.0;
    bool        fadingEnabled = false;
    double      fadingStdDev  = 4.0;
};

static void printUsage() {
    std::cout << R"(
========================================
  SIMULADOR DE HANDOVER - CORE ENGINE
  Discrete Event Simulator (DES) v2.0
========================================

USAGE:
  handover_sim <mode> [options]

MODES:
  synthetic              Simula 3 torres (eNodeBs) com Path Loss Model.
                         UE percorre 200m em linha reta.
  replay <file.json>     Processa dados reais capturados pelo scanner Python.

OPTIONS:
  --ttt <ms>             Time-to-Trigger em milissegundos (3GPP TS 36.331).
                         Valores válidos: 0, 40, 64, 80, 100, 128, 160, 256, 320, 480, 512, 640.
                         Default: 160

  --hysteresis <dB>      Histerese do evento A3 em dB.
                         Controla a margem mínima de sinal para iniciar handover.
                         Default: 3.0

  --fading               Habilita Rayleigh Fading na propagação de sinal.
                         Adiciona variabilidade estocástica ao RSRP calculado.

  --fading-std <dB>      Desvio padrão do shadowing (usado com --fading).
                         Default: 4.0

  --help, -h             Exibe esta mensagem de ajuda.

EXAMPLES:
  handover_sim synthetic
  handover_sim synthetic --ttt 320 --hysteresis 5.0 --fading
  handover_sim replay data/network_log.json
  handover_sim replay data/network_log.json --ttt 100

ABOUT:
  Implementa o algoritmo A3 Event (3GPP TS 36.331) com Time-to-Trigger
  para decisão de handover em redes LTE. O motor DES processa eventos com
  std::priority_queue<Event> e exporta KPIs de telecom em CSV.
)";
}

static SimulationConfig parseArguments(int argc, char* argv[]) {
    SimulationConfig config;

    if (argc < 2) {
        config.mode = "synthetic";
        return config;
    }

    std::string firstArg = argv[1];
    if (firstArg == "--help" || firstArg == "-h") {
        printUsage();
        std::exit(0);
    }

    config.mode = firstArg;

    int i = 2;
    if (config.mode == "replay" && argc > 2) {
        std::string possiblePath = argv[2];
        if (possiblePath.find("--") != 0) {
            config.jsonFilePath = possiblePath;
            i = 3;
        }
    }

    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "--ttt" && i + 1 < argc) {
            config.tttMs = std::stod(argv[++i]);
        } else if (arg == "--hysteresis" && i + 1 < argc) {
            config.hysteresisDb = std::stod(argv[++i]);
        } else if (arg == "--fading") {
            config.fadingEnabled = true;
        } else if (arg == "--fading-std" && i + 1 < argc) {
            config.fadingStdDev = std::stod(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            std::exit(0);
        } else {
            std::cerr << "[WARN] Unknown option: " << arg << "\n";
        }
        ++i;
    }

    return config;
}

static void configureEngine(SimulationEngine& engine, const SimulationConfig& config) {
    engine.setA3Parameters(config.hysteresisDb, config.tttMs / 1000.0);
    engine.setFadingEnabled(config.fadingEnabled, config.fadingStdDev);
}

static void startReplayMode(const SimulationConfig& config) {
    std::cout << "\n  Mode: REAL-DATA REPLAY\n";
    std::cout << "  File: " << config.jsonFilePath << "\n";
    std::cout << "  TTT:  " << config.tttMs << " ms | Hysteresis: " << config.hysteresisDb << " dB\n";

    SimulationEngine engine;
    configureEngine(engine, config);

    auto metrics = std::make_shared<MetricsCollector>();
    engine.addObserver(metrics);

    engine.setMobileNode(MobileNode("UE-001"));
    engine.loadSimulationDataFromJson(config.jsonFilePath);
    engine.startSimulation();
    engine.printSimulationSummary();

    metrics->printKpiSummary();
    metrics->exportToCsv("output");
}

static void startSyntheticMode(const SimulationConfig& config) {
    std::cout << "\n  Mode: SYNTHETIC (GPS + Path Loss Model)\n";
    std::cout << "  TTT:  " << config.tttMs << " ms | Hysteresis: " << config.hysteresisDb << " dB";
    if (config.fadingEnabled) {
        std::cout << " | Fading: ON (σ=" << config.fadingStdDev << " dB)";
    }
    std::cout << "\n\n";

    SimulationEngine engine;
    configureEngine(engine, config);

    auto metrics = std::make_shared<MetricsCollector>();
    engine.addObserver(metrics);

    engine.addBaseStation(BaseStation("eNB-A", 0.0,   0.0,  20.0));
    engine.addBaseStation(BaseStation("eNB-B", 100.0, 0.0,  20.0));
    engine.addBaseStation(BaseStation("eNB-C", 200.0, 0.0,  20.0));

    MobileNode mobileNode("UE-001", 10.0, 0.0);
    engine.setMobileNode(mobileNode);

    double currentTime = 0.0;
    for (double xCoordinate = 10.0; xCoordinate <= 210.0; xCoordinate += 20.0) {
        Event movementEvent{};
        movementEvent.timestampSeconds = currentTime;
        movementEvent.type      = EventType::USER_MOVEMENT;
        movementEvent.newPositionX = xCoordinate;
        movementEvent.newPositionY = 0.0;
        engine.scheduleEvent(movementEvent);
        currentTime += 5.0;

        for (const std::string& stationIdentifier : {"eNB-A", "eNB-B", "eNB-C"}) {
            Event measurementEvent{};
            measurementEvent.timestampSeconds = currentTime - 4.9;
            measurementEvent.type         = EventType::SIGNAL_MEASUREMENT;
            measurementEvent.sourceBaseStationId = stationIdentifier;
            measurementEvent.rsrpValueDbm     = -140.0;
            engine.scheduleEvent(measurementEvent);
        }
    }

    Event simulationEndEvent{};
    simulationEndEvent.timestampSeconds = currentTime;
    simulationEndEvent.type      = EventType::SIMULATION_END;
    engine.scheduleEvent(simulationEndEvent);

    engine.startSimulation();
    engine.printSimulationSummary();

    metrics->printKpiSummary();
    metrics->exportToCsv("output");
}

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "  SIMULADOR DE HANDOVER - CORE ENGINE\n";
    std::cout << "  Discrete Event Simulator (DES) v2.0\n";
    std::cout << "========================================\n";

    SimulationConfig config = parseArguments(argc, argv);

    if (config.mode == "replay") {
        if (!std::filesystem::exists(config.jsonFilePath)) {
            std::cerr << "\n[ERROR] File not found: " << config.jsonFilePath << "\n";
            return 1;
        }
        startReplayMode(config);

    } else if (config.mode == "synthetic") {
        startSyntheticMode(config);

    } else {
        std::cerr << "\n[ERROR] Invalid mode: '" << config.mode << "'\n";
        std::cerr << "Run 'handover_sim --help' for usage.\n";
        return 1;
    }

    return 0;
}
