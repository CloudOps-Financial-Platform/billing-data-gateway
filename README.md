
# Billing Data Gateway

> A high-performance, low-latency C11 cloud billing ingestion engine that normalizes heterogeneous hyperscaler billing exports into a unified Intermediate Financial Model (IFM) using zero-copy POSIX `mmap()` parsing, dynamic schema inversion matrices, and fixed-point currency arithmetic.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-v0.6.0-blue)
![Language](https://img.shields.io/badge/language-C11-blue)
![Memory Model](https://img.shields.io/badge/memory--model-POSIX%20mmap()-orange)
![Test Suite](https://img.shields.io/badge/tests-passing-brightgreen)
![Project Status](https://img.shields.io/badge/status-active%20development-emerald)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 🎯 What Problem Does This Solve?

Enterprise organizations operating at scale receive cloud billing exports from multiple hyperscalers daily. These exports present massive architectural challenges: radically conflicting data schemas, unstable column sequence offsets, multi-gigabyte files, and variations in float decimal formatting that inject systemic rounding leakage into accounting pipelines.

**Billing Data Gateway** provides a zero-dependency, low-level system ingestion core that parses and normalizes disparate vendor exports at high velocity. By dynamically mapping provider variations at the file boundary, the engine translates raw files into an immutable, in-memory array representation. This guarantees high predictability and data consistency for downstream FinOps data warehouses, analytics platforms, and automated cloud accounting tools.

---

## 📐 Architectural Ingestion Flow

The pipeline streams and maps data entirely within virtual memory, prioritizing zero data mutation and avoiding dynamic heap overhead during processing loops.

```text
 Raw Cloud Billing Export CSV (AWS CUR 2.0 / Azure Cost Management)
                           │
                           ▼
         [ POSIX mmap() Virtual File Ingestion ]
                           │
                           ▼
      [ Dynamic Provider Registry Auto-Detection ]
                           │
         ┌─────────────────┴─────────────────┐
         ▼                                   ▼
  [ REJECT: Unknown Schema ]     [ PASS: Vendor Adapter Loaded ]
  (Pipeline Safety Abort)             (AWS / Azure Configured)
                                             │
                                             ▼
                                 [ Header Mapping Resolver ]
                                 (Dynamic Offset Table Inversion)
                                             │
                                             ▼
                                 [ Zero-Copy String Tokenizer ]
                                 (str_slice_t Row Windowing)
                                             │
                                             ▼
                                 [ Fixed-Point Currency Invariant ]
                                 (ASCII parsing straight to micros)
                                             │
                                             ▼
                              [ Normalized In-Memory IFM Record Array ]

```

---

## ⚡ Core Technical Features

* **POSIX `mmap()` File Ingestion:** Projects file descriptors directly into the virtual address space, maximizing kernel-to-userland page fault transfer speeds and bypassing standard buffer copy costs.
* **Zero-Copy Tokenization:** Tracks data row offsets via pointer-and-length structures (`str_slice_t`), completely eradicating internal string copying or `malloc()` performance drops inside loop iterations.
* **Dynamic Provider Registry:** Peeks directly into initial byte streams at runtime to detect vendor schemas, loading matching adapter configurations automatically.
* **Schema Inversion Adapters:** Normalizes structural column drift and sequence variations via dynamic lookup tables, allowing out-of-order field resolution.
* **Fixed-Point Decimal Core:** Parses pricing elements straight out of raw text patterns into `int64_t` micro-units ($1.00 = 1,000,000 µ$), eliminating binary floating-point rounding errors.
* **Defensive Boundary Controls:** Validates internal arrays against dirty headers, multi-declaration fields, and uninspected payloads, failing gracefully through dedicated record isolation flags.

---

## 💡 The Intermediate Financial Model (IFM)

Hyperscalers present structural semantic variations for identical items (e.g., `lineItem/UnblendedCost` in AWS vs. `PreTaxCost` or `costInBillingCurrency` in Azure). The **Intermediate Financial Model (IFM)** defines a consistent, layout-decoupled canonical binary record structure (`ifm_record_t`) to standardize consumption attributes:

```c
typedef struct {
    provider_type_t provider;       /* Hyperscaler identifier signature */
    str_slice_t provider_row_id;    /* Zero-copy reference to row asset index */
    str_slice_t account_id;         /* Direct pointer reference to account allocation context */
    str_slice_t resource_id;        /* Fixed pointer view tracking target cloud infrastructure resource */
    str_slice_t usage_start_raw;    /* Real-world timestamp allocation boundary */
    int64_t billed_cost_micros;     /* Fixed-point integer micro-currency value ($1 = 1,000,000 µ) */
    size_t source_line;             /* Traceable file line tracking array offset */
    record_flags_t flags;           /* Internal validation bitmask markers */
} ifm_record_t;

```

---

## 🌐 Current Provider Support

| Cloud Provider Layout | Ingestion Offset Mapping | Processing Strategy | Status |
| --- | --- | --- | --- |
| **AWS CUR 2.0** | Dynamic Header Offset Array Matrix | Fixed-Point Fractional Cost Invariant | ✅ Active |
| **Azure Cost Management** | Dynamic Header Offset Array Matrix | Fixed-Point Fractional Cost Invariant | ✅ Active |
| **GCP Billing Export** | BigQuery Structured Variant Schema | Chunked Buffer Multi-Stream Extraction | 📅 Planned |

---

## 🖥️ Command-Line Interface

The application features a built-in production CLI built around POSIX standard flag inputs, providing runtime diagnostic telemetry data for automated infrastructure tasks.

### Core Command Operations

* **Display Interface Help:**
```bash
./billing-gateway -h

```


* **Verify Version & Build Metadata:**
```bash
./billing-gateway -v

```


* **Normalize Cloud Billing Payload Export:**
```bash
./billing-gateway -i data/aws_cur_shifted.csv

```



### CLI Terminal Output Snapshot

```text
====================================================
Billing Data Gateway v0.6.0
====================================================
Input File        : data/aws_cur_shifted.csv
File Size         : 407 B
Provider          : AWS (CUR 2.0)
Records Processed : 3
Elapsed Time      : 10.07 ms (0.0101 seconds)
Throughput        : 298 rows/sec

Exit Code         : 0
====================================================

```

---

## 📊 Performance Benchmarks (v0.5.0 Baseline)

Profiles are compiled using a high-resolution, hardware-isolated tracking framework using POSIX `CLOCK_MONOTONIC` to record execution windows with nanosecond precision, bypassing system execution jitters.

### Test Environment Profile

* **Hardware Compute Grid:** Acer Nitro V 16 Core Processing Unit
* **Operating System Node:** WSL2 Virtual Subsystem Matrix (Ubuntu Linux Environment)
* **Compiler Stack Ingress:** GCC C11 Target Profile Configurations (`-Wall -Wextra -O3`)
* **Test Dataset Workload:** 100,000 Synchronous AWS CUR 2.0 Data Rows (6.54 MB payload)

### Empirical Telemetry Performance Data

* **Data Processing Bandwidth:** 35.49 MB/sec
* **Ingestion Pipeline Velocity:** 542,535 records/second
* **Total Pipeline Ingestion Window:** 184.32 ms
* **Average Latency Overhead Cost:** 1,843.20 nanoseconds / record

---

## 📁 Repository Layout

The workspace partitions operational ingestion engines from data testing targets:

```text
billing-data-gateway/
├── include/
│   ├── ifm_spec.h              # Canonical primitives and record layout schemas
│   ├── mmap_reader.h           # POSIX memory-mapped paging system definitions
│   ├── provider_adapters.h     # Hyperscaler mapping interface specifications
│   ├── provider_registry.h     # Dynamic routing registration maps
│   ├── pipeline.h              # End-to-end processing orchestration definitions
│   └── version.h               # Central release version and timestamp parameters
├── src/
│   ├── mmap_reader.c           # Zero-copy memory page mapping layer
│   ├── adapters/
│   │   ├── aws_adapter.c       # AWS CUR 2.0 dynamic schema mapping logic
│   │   └── azure_adapter.c     # Azure cost normalization structural mapping logic
│   ├── providers/
│   │   └── provider_registry.c # File header lookup scanner and routing core
│   ├── utils/
│   │   └── fixed_point.c       # Fast integer micro-currency math components
│   ├── pipeline.c              # Core processing loop manager
│   └── main.c                  # Command-line input processing wrapper
├── tests/
│   └── test_pipeline.c        # End-to-end validation test configurations
├── benchmarks/
│   └── bench_throughput.c      # POSIX timer performance tracking framework
├── data/
│   └── aws_cur_shifted.csv     # Mock billing records used during sprint runs
└── README.md                   # Permanent asset architecture documentation

```

---

## ⚡ Quick Start

### 1. Clone the Source Repository

```bash
git clone [https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git](https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git)
cd billing-data-gateway

```

### 2. Compile Validation Core Test Assemblies

```bash
gcc -Wall -Wextra -O3 -Iinclude \
    src/mmap_reader.c \
    src/utils/fixed_point.c \
    src/adapters/aws_adapter.c \
    src/adapters/azure_adapter.c \
    src/providers/provider_registry.c \
    src/pipeline.c \
    tests/test_pipeline.c \
    -o test_runner

```

### 3. Run E2E Test Suite

```bash
./test_runner

```

### 4. Compile and Run High-Resolution Throughput Benchmarks

```bash
gcc -Wall -Wextra -O3 -Iinclude \
    src/mmap_reader.c \
    src/utils/fixed_point.c \
    src/adapters/aws_adapter.c \
    src/adapters/azure_adapter.c \
    src/providers/provider_registry.c \
    src/pipeline.c \
    benchmarks/bench_throughput.c \
    -o bench_runner && ./bench_runner

```

---

## 🗺️ Project Execution Timeline

* ✅ **v0.5.0** — Core normalizer engine, `mmap()` file mappings, fixed-point math loops, and monotonic benchmarking telemetry arrays.
* ✅ **v0.6.0** — Built-in POSIX `getopt()` CLI, version checks, file metadata parsing, dynamic size scaling, and verification error isolation loops.
* 🔄 **v0.7.0 [Sprint 2 Target]** — High-Throughput Structured Output Serializers (Streaming JSON / Apache Parquet / Apache Arrow extraction arrays).
* 🎯 **v1.0.0** — Production stable iteration deployment interface baseline.

---

## 📜 Historical Release Ledger

| Version | Milestone Date | Core Technical Highlights Deliverables | Ingestion Baseline |
| --- | --- | --- | --- |
| **`v0.6.0`** | July 28, 2026 | Standalone terminal CLI, version mappings, dynamic size formats, truth data summaries. | Verified Tested Sets |
| **`v0.5.0`** | July 27, 2026 | Embedded high-res timer benchmarks, 100k data array load loops, throughput diagnostics. | Baseline Fact |

---

## 📄 License

This repository is open-source infrastructure software released under the terms of the MIT License.

```

```