## 🤖 Mapeamento para o Ecossistema Android

Este projeto foi arquitetado espelhando o **Android Telephony Stack**, a camada do sistema responsável por gerenciar o rádio do celular.

### Como o Android Gerencia o Sinal de Rede

```
┌──────────────────────────────────────────────────────────┐
│                   ANDROID FRAMEWORK (Java)               │
│  TelephonyManager.getSignalStrength()                    │
│  CellInfoLte.getCellSignalStrengthLte().getRsrp()        │
└──────────────────┬───────────────────────────────────────┘
                   │  JNI (Java Native Interface)
                   ▼
┌──────────────────────────────────────────────────────────┐
│               RADIO INTERFACE LAYER - RIL (C++)          │
│  libril.so → rild daemon → SignalFilter equivalente      │
└──────────────────┬───────────────────────────────────────┘
                   │  HAL (Hardware Abstraction Layer)
                   ▼
┌──────────────────────────────────────────────────────────┐
│               CHIP DE RÁDIO (Modem / Baseband)           │
│  Qualcomm / MediaTek → transmite RSRP bruto              │
└──────────────────────────────────────────────────────────┘
```

### Equivalências: Projeto ↔ Android Real

| Este Projeto | Android / 3GPP |
|---|---|
| `MobileNode` | `TelephonyManager.getSignalStrength()` |
| `BaseStation (eNodeB)` | `CellInfoLte.getCellSignalStrengthLte()` |
| `SignalFilter` (Média Móvel) | Filtro interno do `RIL Daemon` |
| `SimulationEngine` (DES) | Simulador interno de rede (`ns-3`) |
| `A3 Event + TTT` | 3GPP TS 36.331 — padrão LTE real |
| `HandoverJNI.java + handover_jni.cpp` | Padrão JNI do Android NDK |

### A Ponte JNI (`jni/`)

A pasta `jni/` demonstra como o Android conecta o **Java Framework** às **bibliotecas nativas C++**:

```java
HandoverJNI jni = new HandoverJNI();
double filteredRsrp = jni.computeFilteredRsrp(rawReadings, 5);
boolean handoverNeeded = jni.isA3EventConditionMet(neighborRsrp, servingRsrp, 3.0);
```

Isso espelha o que acontece no Android quando o `TelephonyManager` recebe uma medição do modem: ele chama a `libril.so` (escrita em C++) via JNI para processar os dados antes de expô-los ao app.

