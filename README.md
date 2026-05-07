# 📡 Simulador de Handover — Discrete Event Simulator

> Demonstrador técnico de competência em **Sistemas de Telecomunicações**, **C++ moderno**, **Python**, **JNI/Android** e práticas de **Engenharia de Software**.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![Google Test](https://img.shields.io/badge/tests-Google%20Test-4285F4.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## 📋 Visão Geral

Sistema híbrido que une **captura de dados reais de rede Wi-Fi** com um **motor de simulação de eventos discretos (DES)** para validar o algoritmo de Handover — o mecanismo que permite a um celular trocar de torre (eNodeB) sem perda de conexão.

```
┌─────────────────────┐     JSON      ┌──────────────────────────┐
│   Scanner Python    │ ──────────►  │   Simulador C++ (DES)    │
│   (netsh / Wi-Fi)   │              │   - Fila de Prioridade   │
│   SSID, BSSID, RSSI │              │   - Filtro de Sinal      │
└─────────────────────┘              │   - Lógica de Handover   │
                                     │   - Métricas / KPIs      │
                                     └──────────┬───────────────┘
                                                │  CSV
                                     ┌──────────▼───────────────┐
                                     │   Dashboard Python       │
                                     │   (matplotlib/pandas)    │
                                     └──────────────────────────┘
```

---

## 🏗️ Arquitetura

| Camada | Linguagem | Responsabilidade |
|--------|-----------|-----------------:|
| Scanner | Python | Captura BSSID/RSSI via `netsh` (Windows) |
| Filtro de Sinal | C++ | Suaviza oscilações com Média Móvel (Buffer Circular) |
| Motor DES | C++ | Processa eventos com `std::priority_queue` |
| Lógica de Handover | C++ | Decisão com **A3 Event + TTT** (3GPP TS 36.331) |
| Propagação RF | C++ | **Log-Distance Path Loss** + **Rayleigh Fading** |
| Métricas / KPIs | C++ | Observer Pattern → CSV export |
| Ponte Nativa | C++/Java | **JNI** — espelha Android RIL |
| Dashboard | Python | **matplotlib** — gráficos profissionais |
| Testes | C++ | **Google Test** — testes unitários e integração |
| CI/CD | YAML | GitHub Actions (matrix build, sanitizers, coverage) |
| Containerização | Docker | Multi-stage build para reprodutibilidade |

---

## 🚀 Como Executar

### Pré-requisitos
- Python 3.10+
- CMake 3.16+ e compilador C++17 (GCC, Clang ou MSVC)
- Docker (opcional)

---

### ▶️ Fase 1: Capturar Dados Reais (Windows)

```powershell
cd scanner
pip install -r requirements.txt
python main.py --interval 3 --count 30
```

Isso gera o arquivo `data/network_log.json` com 30 varreduras Wi-Fi.

---

### ▶️ Fase 2A: Simulador (Modo Replay — Dados Reais)

```bash
cd simulator
cmake -B build .
cmake --build build

# Windows:
.\build\handover_sim.exe replay ../data/network_log.json

# Linux/macOS:
./build/handover_sim replay ../data/network_log.json
```

### ▶️ Fase 2B: Simulador (Modo Sintético — GPS/Path Loss)

```bash
./build/handover_sim synthetic
```

O UE percorre 200 metros entre 3 torres (eNodeBs), e o sinal é calculado via **Log-Distance Path Loss Model** com **Rayleigh Fading** opcional:

```
RSRP(d) = TxPower - 10 × n × log₁₀(d) + X_σ
          └─── Path Loss ───┘             └ Fading
```

### ▶️ Parâmetros de Simulação

O simulador aceita parâmetros via linha de comando para ajuste fino:

```bash
# Consultar todas as opções disponíveis:
./build/handover_sim --help

# Personalizar TTT e Histerese (valores reais da 3GPP):
./build/handover_sim synthetic --ttt 320 --hysteresis 5.0

# Habilitar Rayleigh Fading com desvio customizado:
./build/handover_sim synthetic --fading --fading-std 6.0

# Replay com TTT mais agressivo:
./build/handover_sim replay data/network_log.json --ttt 80
```

| Parâmetro | Default | Descrição |
|---|---|---|
| `--ttt <ms>` | 160 | Time-to-Trigger (3GPP TS 36.331) |
| `--hysteresis <dB>` | 3.0 | Margem de histerese do evento A3 |
| `--fading` | OFF | Habilita Rayleigh Fading |
| `--fading-std <dB>` | 4.0 | Desvio padrão do shadowing |

---

### ▶️ Fase 3: Testes & Análise

**Testes unitários + integração (Google Test):**
```bash
cd simulator/build && ctest --output-on-failure --verbose
```

**Dashboard de visualização:**
```bash
pip install -r analysis/requirements.txt
python analysis/plot_handover.py --input-dir output
```

**Docker (modo completo):**
```bash
docker build -t handover-sim .
docker run --rm handover-sim
```

---

## 🧪 Suite de Testes

| Arquivo | Testes | O que valida |
|---------|:------:|-----------:|
| `test_signal_filter` | 10 | Buffer circular, média móvel, reset, edge cases |
| `test_a3_event` | 9 | Condição A3 (3GPP), histerese, thresholds |
| `test_ttt_logic` | 4 | Time-to-Trigger, reset de timer, multi-candidato |
| `test_path_loss` | 8 | Log-Distance, clamping, expoentes, monotonicidade |
| `test_path_loss` (fading) | 2 | Rayleigh fading: variabilidade, média estatística |
| `test_path_loss` (distance) | 3 | Distância euclidiana |
| `test_simulation_engine` | 7 | Integração end-to-end, métricas, cobertura |
| `test_ping_pong` | 3 | Anti ping-pong, taxa de ping-pong, transição limpa |

**Ferramentas de qualidade no CI:**
- ✅ **Google Test** — Framework de testes industrial
- ✅ **ASan + UBSan** — Memory leaks, buffer overflow, undefined behavior
- ✅ **Code Coverage** (gcov/lcov) — Relatório de cobertura
- ✅ **Matrix Build** — GCC + Clang

---

## 📊 KPIs de Telecom Exportados

O simulador calcula e exporta métricas profissionais de rede:

| KPI | Descrição |
|-----|-----------:|
| **Total Handovers** | Número de handovers executados |
| **Ping-Pong Rate (%)** | % de HOs que retornam à célula anterior em < 5s |
| **Average RSRP (dBm)** | Intensidade média do sinal na simulação |
| **Coverage (%)** | % do tempo com RSRP > -110 dBm |

Arquivos CSV exportados em `output/`:
```
output/
├── simulation_metrics.csv    # KPIs resumidos
├── handover_events.csv       # Log detalhado de cada handover
├── signal_trace.csv          # RSRP de cada torre por timestep
└── position_trace.csv        # Trajetória do UE
```

---

## 🧠 Conceitos Técnicos Demonstrados

### C++ Moderno e Telecomunicações
- **Buffer Circular com Ponteiro Raw** (`double*`): Simula a técnica usada em drivers de modem para bufferização de pacotes sem overhead de alocação dinâmica.
- **RAII**: Gerenciamento de memória com destrutor explícito (`delete[]`), fundamental em sistemas que rodam 24/7.
- **`std::priority_queue`**: Fila de eventos DES com complexidade O(log N), padrão em simuladores como o **ns-3**.
- **Smart Pointers** (`unique_ptr`): Propriedade exclusiva do `MobileNode`, evitando leaks em sistemas de longa duração.
- **Observer Pattern** (GoF): Desacopla a simulação dos coletores de métricas — análogo ao `PhoneStateListener` do Android.
- **`nlohmann/json`**: Parsing robusto de JSON com tratamento de erros.

### Algoritmo de Handover (3GPP TS 36.331)
```
Condição A3: RSRP_candidato > RSRP_serving + Histerese (3 dB)
Time-to-Trigger: Condição deve persistir por 160ms antes do handover
```
A **histerese** + **TTT** evitam o *ping-pong effect* — situação onde o celular fica alternando entre duas torres que têm sinais parecidos.

### Propagação de RF
```
Log-Distance: RSRP(d) = TxPower - 10 × n × log₁₀(d)
Rayleigh:     RSRP_faded = RSRP_base + N(0, σ²)   [σ = 4 dB típico]
```

---

## 📁 Estrutura do Projeto

```
handover-simulator/
├── scanner/
│   ├── main.py              # Entry point com CLI
│   ├── data_logger.py       # Captura e log de dados Wi-Fi
│   └── requirements.txt
├── simulator/
│   ├── include/
│   │   ├── Event.h              # Tipos de eventos DES
│   │   ├── BaseStation.h        # Entidade eNodeB
│   │   ├── MobileNode.h         # Entidade UE (celular)
│   │   ├── SignalFilter.h       # Filtro de Média Móvel
│   │   ├── SimulationEngine.h   # Motor DES + free functions
│   │   ├── SimulationObserver.h # Interface Observer
│   │   └── MetricsCollector.h   # Coletor de KPIs
│   ├── src/
│   │   ├── SignalFilter.cpp
│   │   ├── SimulationEngine.cpp
│   │   ├── MetricsCollector.cpp
│   │   └── main.cpp
│   ├── tests/
│   │   ├── test_signal_filter.cpp
│   │   ├── test_a3_event.cpp
│   │   ├── test_ttt_logic.cpp
│   │   ├── test_path_loss.cpp
│   │   ├── test_simulation_engine.cpp
│   │   └── test_ping_pong.cpp
│   └── CMakeLists.txt
├── jni/
│   ├── HandoverJNI.java     # Ponte Java → C++ (padrão Android NDK)
│   └── handover_jni.cpp     # Implementação nativa JNI
├── analysis/
│   ├── plot_handover.py     # Dashboard matplotlib
│   └── requirements.txt
├── data/                    # Logs JSON (gerados pelo scanner)
├── docs/
│   └── Doxyfile             # Configuração para API docs
├── Dockerfile               # Build multi-stage
├── .github/workflows/ci.yml # Pipeline CI/CD (5 jobs)
├── .gitignore
└── LICENSE                  # MIT License
```

---

## 📊 Exemplo de Saída

```
========================================
  SIMULADOR DE HANDOVER - CORE ENGINE
  Discrete Event Simulator (DES) v2.0
========================================

  Mode: SYNTHETIC (GPS + Path Loss Model)
  TTT:  160 ms | Hysteresis: 3 dB

[DES] ===== STARTING SIMULATION (3GPP A3 Event + TTT=160ms, Hys=3dB) =====
[DES] UE: UE-001 | Connected to: eNB-A

[DES] t=0.0s  | MOVEMENT  | UE at (10.0, 0.0)
[DES] t=5.0s  | MOVEMENT  | UE at (30.0, 0.0)
[DES] t=10.0s | *** A3 HANDOVER *** | eNB-A -> eNB-B (RSRP: -58.3 dBm)
[DES] t=15.0s | *** A3 HANDOVER *** | eNB-B -> eNB-C (RSRP: -55.1 dBm)
[DES] t=40.0s | SIMULATION_END

[DES] ===== SIMULATION SUMMARY =====
[DES] Total A3 Handovers : 2
[DES] Stations detected  : 3

[KPI] ===== TELECOM KPIs =====
[KPI] Total Handovers     : 2
[KPI] Ping-Pong Rate      : 0.0 %
[KPI] Average RSRP        : -62.4 dBm
[KPI] Coverage (>-110 dBm): 100.0 %
[KPI] ============================

[METRICS] CSV files exported to: output/
```

---

*Desenvolvido como projeto técnico para demonstração de conhecimentos em redes móveis, C++ de sistemas e engenharia Android.*
