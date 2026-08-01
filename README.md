
# Billing Data Gateway

> A high-performance, low-latency C11 cloud billing ingestion engine that normalizes heterogeneous hyperscaler billing exports into a unified Intermediate Financial Model (IFM) using zero-copy POSIX `mmap()` parsing, dynamic schema inversion matrices, bitmask validation, and fixed-point currency arithmetic.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-v1.0.0-blue)
![Language](https://img.shields.io/badge/language-C11-blue)
![Memory Model](https://img.shields.io/badge/memory--model-POSIX%20mmap()-orange)
![Test Suite](https://img.shields.io/badge/tests-passing-brightgreen)
![Project Status](https://img.shields.io/badge/status-production%20release-emerald)
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
          ┌──────────────────┴──────────────────┐
          ▼                                     ▼
   [ REJECT: Unknown Schema ]     [ PASS: Vendor Adapter Loaded ]
   (Pipeline Safety Abort)         (AWS / Azure Configured)
                                                │
                                                ▼
                                   [ Header Mapping Resolver ]
                                   (Dynamic Offset Table & Aliasing)
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
                                   [ System 5 Validation Engine ]
                                   (Bitmask Flags: RECORD_VALID, etc.)
                                                │
                                                ▼
                               [ Normalized In-Memory IFM Record Array ]
                                                │
                                                ▼
                               [ Output Serialization Engine ]
                               (Streaming JSON / Canonical CSV)
                                                │
                                                ▼
                                       stdout / File / Pipe

```

---

## ⚡ Core Technical Features

* **POSIX `mmap()` File Ingestion:** Projects file descriptors directly into the virtual address space, maximizing kernel-to-userland page fault transfer speeds and bypassing standard buffer copy costs.
* **Zero-Copy Tokenization:** Tracks data row offsets via pointer-and-length structures (`str_slice_t`), completely eradicating internal string copying or `malloc()` performance drops inside loop iterations.
* **Dynamic Provider Registry:** Peeks directly into initial byte streams at runtime to detect vendor schemas, loading matching adapter configurations automatically.
* **Schema Inversion & Header Aliasing:** Normalizes structural column drift, header sequence variations, and vendor column aliases (e.g., `lineItem/UsageAccountId` vs `lineItem/AccountId`).
* **Fixed-Point Decimal Core:** Parses pricing elements straight out of raw text patterns into `int64_t` micro-units ($1.00 = 1,000,000 µ$), eliminating binary floating-point rounding errors.
* **System 5 Validation Engine:** Evaluates record integrity at runtime, setting bitmask flags for missing primary keys, invalid dates, negative costs, or empty resource IDs.
* **Dual Format Streaming Serializer:** Streams normalized IFM records directly to `stdout` or file descriptors in both **JSON** and **Canonical CSV** formats without copying intermediate record structures.
* **Defensive Boundary Controls:** Validates internal arrays against dirty headers, multi-declaration fields, and uninspected payloads, failing gracefully through dedicated record isolation flags.

---

## 💡 The Intermediate Financial Model (IFM)

Hyperscalers present structural semantic variations for identical items (e.g., `lineItem/UnblendedCost` in AWS vs. `PreTaxCost` or `costInBillingCurrency` in Azure). The **Intermediate Financial Model (IFM)** defines a consistent, layout-decoupled canonical binary record structure (`ifm_record_t`) to standardize consumption attributes:

```c
typedef struct {
    size_t source_line;             /* Traceable file line tracking array offset */
    provider_type_t provider;       /* Hyperscaler identifier signature */
    record_flags_t flags;           /* Internal validation bitmask markers */
    str_slice_t provider_row_id;    /* Zero-copy reference to row asset index */
    str_slice_t account_id;         /* Direct pointer reference to account allocation context */
    str_slice_t resource_id;        /* Fixed pointer view tracking target cloud infrastructure resource */
    str_slice_t usage_start_raw;    /* Real-world timestamp allocation boundary */
    int64_t billed_cost_micros;     /* Fixed-point integer micro-currency value ($1 = 1,000,000 µ) */
} ifm_record_t;

```

### Validation Bitmask Flag Matrix (`record_flags_t`)

| Bitmask Value | Enum Name | Condition Triggered |
| --- | --- | --- |
| `0x00` (`0`) | `RECORD_VALID` | Passed all structural and field validation assertions. |
| `0x01` (`1`) | `RECORD_MISSING_KEY` | Missing or empty vendor Account / Subscription ID. |
| `0x02` (`2`) | `RECORD_INVALID_COST` | Cost metric parsed as non-numeric, negative, or invalid. |
| `0x04` (`4`) | `RECORD_INVALID_DATE` | Usage timestamp absent or fails ISO-8601 formatting. |
| `0x08` (`8`) | `RECORD_EMPTY_RESOURCE` | Resource / Asset ID string absent in target column. |

---

## 🌐 Current Provider Support

| Cloud Provider Layout | Ingestion Offset Mapping | Processing Strategy | Status |
| --- | --- | --- | --- |
| **AWS CUR 2.0** | Dynamic Header Offset & Alias Array Matrix | Fixed-Point Fractional Cost Invariant | ✅ Active |
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

* **Normalize Cloud Billing Payload Export (Standard Telemetry Summary):**

```bash
./billing-gateway -i data/aws_cur_shifted.csv

```

* **Stream Normalized Output directly to JSON format (`-f json`):**

```bash
./billing-gateway -i data/aws_cur_shifted.csv -f json

```

* **Stream Normalized Output directly to Canonical CSV format (`-f csv`):**

```bash
./billing-gateway -i data/aws_cur_shifted.csv -f csv

```

### CLI Terminal Output Snapshot (Standard Telemetry)

```text
====================================================
Billing Data Gateway v1.0.0
====================================================
Input File        : data/aws_cur_shifted.csv
File Size         : 407 B
Provider          : AWS (CUR 2.0)
Records Processed : 3
Elapsed Time      : 10.07 ms
Throughput        : 298 rows/sec

====================================================

```

### JSON Output Example (`-f json`)

```json
[
  {
    "source_line": 2,
    "provider": "AWS_CUR",
    "flags": 8,
    "account_id": "acc-998877",
    "resource_id": "",
    "usage_start_raw": "1700000000",
    "billed_cost": 45.800000
  },
  {
    "source_line": 3,
    "provider": "AWS_CUR",
    "flags": 8,
    "account_id": "acc-112233",
    "resource_id": "",
    "usage_start_raw": "1700000000",
    "billed_cost": 120.500000
  }
]

```

### Canonical CSV Output Example (`-f csv`)

```csv
source_line,provider,flags,account_id,resource_id,usage_start_raw,billed_cost
2,AWS_CUR,8,acc-998877,,1700000000,45.800000
3,AWS_CUR,8,acc-112233,,1700000000,120.500000
4,AWS_CUR,8,acc-445566,,1700000000,88.100000

```

---

## ⚙️ Example UNIX/Linux Pipeline Integration

Because the serialization sub-system outputs structured payloads directly to `stdout`, it integrates natively with UNIX utilities like `jq` or can be piped into downstream analytical tools:

```bash
# Formats and colorizes JSON output using jq
./billing-gateway -i data/aws_cur_shifted.csv -f json | jq .

# Streams canonical CSV directly into a data warehouse loader or pipe
./billing-gateway -i data/aws_cur_shifted.csv -f csv | gzip > normalized_billing.csv.gz

```

---

## 📊 Performance Benchmarks (v1.0.0 Verified Telemetry)

Profiles are compiled using a hardware-isolated tracking framework using POSIX `CLOCK_MONOTONIC` to record execution windows with microsecond precision.

### Test Environment Profile

* **Hardware:** Acer Nitro V 16 Core Processing Unit
* **Operating System:** WSL2 Subsystem (Ubuntu Linux Environment)
* **Compiler:** GCC C11 Target Profile Configurations (`-Wall -Wextra -O3`)
* **Test Dataset Workload:** 100,000 Synchronous AWS CUR Data Rows (6.54 MB synthetic payload)

### Empirical Telemetry Performance Data

```text
-----------------------------------------------------------------
 📊 EMPIRICAL TELEMETRY RESULTS (v1.0.0 Verified):
-----------------------------------------------------------------
  • Total Dataset Size   : 6.54 MB (6,860,001 bytes)
  • Records Parsed       : 100,000 / 100,000
  • Total Execution Time : 0.2052 seconds (205.16 ms)
  • Processing Speed     : 31.89 MB/sec
  • Throughput Velocity  : 487,421 records/sec
  • Latency Cost         : 2,051.62 nanoseconds / record
-----------------------------------------------------------------

```

---

## 🛠️ Memory Lifetime & Zero-Copy Architecture

During engineering of the serialization layer, memory lifetime boundaries were hardened: `mmap` page allocations are handled directly at the root application entry point (`main`), guaranteeing that mapped memory pages remain valid throughout all parsing, validation, and serialization loops without triggering buffer copies or premature unmapping page faults.

---

## 📁 Repository Layout

The workspace partitions operational ingestion engines from data testing targets:

```text
billing-data-gateway/
├── docs/
│   └── PRODUCT_ARCHITECTURE_v0.1.md # Architectural requirements & layout specifications
├── include/
│   ├── ifm_spec.h               # Canonical IFM primitives and bitmask definitions
│   ├── mmap_reader.h            # POSIX memory-mapped file definitions
│   ├── provider_adapters.h      # Hyperscaler adapter contract specifications
│   ├── provider_registry.h      # Dynamic provider lookup tables
│   ├── validation.h             # System 5 Validation Engine interface
│   ├── serializer.h             # JSON and CSV output stream signatures
│   ├── pipeline.h               # Orchestration pipeline interface
│   └── version.h                # Gateway version header (v1.0.0)
├── src/
│   ├── mmap_reader.c            # Zero-copy POSIX mmap implementation
│   ├── validation.c            # System 5 Validation Engine rules
│   ├── adapters/
│   │   ├── aws_adapter.c        # AWS CUR schema mapper & header aliasing
│   │   └── azure_adapter.c      # Azure cost normalization logic
│   ├── providers/
│   │   └── provider_registry.c  # Header signature scanner & adapter loader
│   ├── utils/
│   │   ├── fixed_point.c        # Fast integer micro-currency math
│   │   └── serializer.c         # JSON & CSV streaming serialization core
│   ├── pipeline.c               # Stream execution loop manager
│   └── main.c                   # POSIX getopt CLI entrypoint
├── tests/
│   └── test_pipeline.c          # End-to-end regression & validation test harness
├── benchmarks/
│   └── bench_throughput.c       # High-volume stress benchmark harness
├── data/
│   ├── aws_cur_shifted.csv      # Sample multi-cloud billing export
│   └── stress/                  # Malformed test targets for validation testing
├── CHANGELOG.md                 # Production release release notes
└── README.md                    # Asset documentation

```

---

## ⚡ Quick Start

### 1. Clone the Source Repository

```bash
git clone [https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git](https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git)
cd billing-data-gateway

```

### 2. Compile & Run Automated Test Suite (System 8)

```bash
gcc -Wall -Wextra -O3 -Iinclude \
    src/mmap_reader.c \
    src/utils/fixed_point.c \
    src/utils/serializer.c \
    src/adapters/aws_adapter.c \
    src/adapters/azure_adapter.c \
    src/providers/provider_registry.c \
    src/validation.c \
    src/pipeline.c \
    tests/test_pipeline.c \
    -o test_runner

./test_runner

```

### 3. Compile & Run Benchmark Harness (System 9)

```bash
gcc -Wall -Wextra -O3 -Iinclude \
    src/mmap_reader.c \
    src/utils/fixed_point.c \
    src/utils/serializer.c \
    src/adapters/aws_adapter.c \
    src/adapters/azure_adapter.c \
    src/providers/provider_registry.c \
    src/validation.c \
    src/pipeline.c \
    benchmarks/bench_throughput.c \
    -o bench_runner

./bench_runner

```

### 4. Compile & Run Production Gateway CLI

```bash
gcc -Wall -Wextra -O3 -Iinclude \
    src/mmap_reader.c \
    src/utils/fixed_point.c \
    src/utils/serializer.c \
    src/adapters/aws_adapter.c \
    src/adapters/azure_adapter.c \
    src/providers/provider_registry.c \
    src/validation.c \
    src/pipeline.c \
    src/main.c \
    -o billing-gateway

./billing-gateway -i data/aws_cur_shifted.csv -f json

```

---



## 📄 License

This repository is open-source infrastructure software released under the terms of the **MIT License**.

```

