# Simulador de Algoritmos de Calendarización y Mecanismos de Sincronización

Este proyecto fue desarrollado para el curso de **Sistemas Operativos** y simula visualmente distintos algoritmos de calendarización de procesos y mecanismos de sincronización utilizando **C++ y Qt**.

## 📦 Estructura del Proyecto

```bash
.
├── data/                        # Archivos de entrada de ejemplo (.txt)
│
├── process/                    # Estructura de procesos
│   ├── process.cpp             # Implementación de la clase Proceso
│   └── process.h               # Definición de la clase Proceso
│
├── scheduler/                  # Algoritmos de planificación
│   ├── scheduler.h             # Clase abstracta base para todos los algoritmos
│   ├── FIFO/
│   │   ├── fifo_scheduler.cpp  # Implementación del algoritmo FIFO
│   │   └── fifo_scheduler.h
│   ├── Priority/
│   │   ├── priority_scheduler.cpp # Implementación del algoritmo priority
│   │   └── priority_scheduler.h
│   ├── Round Robin/
│   │   ├── rr_scheduler.cpp # Implementación del algoritmo Round Robin
│   │   └── rr_scheduler.h
│   ├── SJF/
│   │   ├── sjf_scheduler.cpp # Implementación del algoritmo SJF
│   │   └── sjf_scheduler.h
│   └── SRTF/
│       ├── srtf_scheduler.cpp # Implementación del algoritmo SRTF
│       └── srtf_scheduler.h
│
├── synchronizer/              # Mecanismos de sincronización
│   ├── synchronizer.h         # Clase base de sincronización
│   ├── mutex_sync.h           # Sincronización con mutex
│   ├── mutex_sync.cpp
│   ├── semaphore_sync.h       # Sincronización con semáforo
│   └── semaphore_sync.cpp
│
├── utils/                     # Utilidades comunes
│   ├── file_loader.h          # Lectura de archivos de entrada
│   ├── file_loader.cpp
│   ├── metrics.h              # Cálculo de métricas como Avg Waiting Time
│   └── metrics.cpp
│
├── visualization/Calendarizer # Interfaz visual Qt
|   ├── Calendarizer.pro       # Archivo de proyecto Qt
│   ├── main.cpp               # Punto de entrada de la aplicación
│   ├── mainwindow.ui          # Diseño de la ventana principal (Qt Designer)
│   ├── mainwindow.h           # Lógica de control de la GUI
│   └── mainwindow.cpp
│
└── .gitignore
```

## ▶️ Cómo abrir y ejecutar el proyecto en Qt Creator

1. Abre Qt Creator.
2. Haz clic en File > Open File or Project...
3. Selecciona el archivo Calendarizer.pro dentro del directorio visualization del proyecto.
4. Qt detectará automáticamente la configuración. Elige tu compilador preferido (GCC, MSVC, etc.) y haz clic en Configure Project.
5. Usa el botón verde Run (▶️) para compilar y ejecutar la aplicación.

## ✅ Funcionalidades principales
- Simulación dinámica de los siguientes algoritmos:
  - First In First Out (FIFO)
  - Shortest Job First (SJF)
  - Shortest Remaining Time First (SRTF)
  - Round Robin (con quantum configurable)
  - Priority

- Simulación de sincronización usando:
  - Mutex
  - Semáforos

- Visualización tipo Gantt animada por ciclo
- Scroll horizontal dinámico
- Resumen de métricas de eficiencia












