# Billing Data Gateway

> A high-performance C11 cloud billing normalization engine that converts heterogeneous cloud provider billing exports into a unified Intermediate Financial Model (IFM) using zero-copy POSIX mmap() parsing, dynamic schema resolution, and fixed-point currency arithmetic.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-v0.5.0-blue)
![Language](https://img.shields.io/badge/language-C11-blue)
![Memory Model](https://img.shields.io/badge/memory--model-POSIX%20mmap()-orange)
![Test Suite](https://img.shields.io/badge/tests-passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 🎯 What Problem Does This Solve?

Enterprise organizations frequently receive billing exports from multiple cloud providers, each with radically different schemas, changing column positions, and inconsistent currency representations. 

Billing Data Gateway provides a reusable, low-level ingestion layer that normalizes these diverse formats at high velocity. It resolves vendor header variances at runtime, producing a canonical data structure for downstream FinOps analytics, data warehouses, and reporting applications without introducing heap fragmentation or precision errors.

---

## 📐 Architectural Ingestion Flow

The pipeline maps, parses, and normalizes file byte streams completely within virtual memory.

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
 (Pipeline Safety Abort)                    │
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
⚡ FeaturesPOSIX mmap() Ingestion: Maps file descriptors directly into the process virtual address space, bypassing kernel-to-userland buffer copy costs.Zero-Copy String Handling: Uses lightweight pointer-and-length structures (str_slice_t) to parse rows without mutating data or calling malloc inside the data stream.Dynamic Provider Registry: Automatically inspects header signatures to discover hyperscaler formats instantly at the file boundary.Runtime Schema Normalization: Resolves structural column drift dynamically via a vendor-decoupled adapter matrix.Fixed-Point Arithmetic: Converts price strings directly into int64_t micro-units ($1.00 = 1,000,000 µ$), eliminating IEEE 754 binary floating-point rounding discrepancies.Defensive Exception Invariants: Catches malformed fields, out-of-bounds metrics, or missing keys, isolating errors cleanly into structured row flags.💡 The Intermediate Financial Model (IFM)Cloud vendors format exported columns under heavily conflicting definitions (lineItem/UnblendedCost in AWS vs. costInBillingCurrency or PreTaxCost in Azure).The Intermediate Financial Model (IFM) defines a consistent canonical binary record (ifm_record_t). The engine maps dynamic structural arrays out-of-order into this standard representation:Ctypedef struct {
    provider_type_t provider;       // Provider Signature Identifiers
    str_slice_t provider_row_id;   // In-place pointer to row item id
    str_slice_t account_id;         // Consolidated account/subscription layout
    str_slice_t resource_id;        // Target execution resource identifier
    str_slice_t usage_start_raw;    // Telemetry window timing coordinate
    int64_t billed_cost_micros;     // Fixed-point exact integer micro-units
    size_t source_line;             // File index location tracker
    record_flags_t flags;           // Structural verification bitmask
} ifm_record_t;
Every downstream pipeline processor or core analytical application operates exclusively on this normalized structure.🌐 Current Provider SupportCloud Provider LayoutIngestion Offset MappingProcessing StrategyStatusAWS CUR 2.0Dynamic Header Columns MatrixMicro-Currency Resolution✅ ActiveAzure Cost ManagementDynamic Header Columns MatrixMicro-Currency Resolution✅ ActiveGCP Billing ExportBigQuery Schema Variant ArrayChunked File Extraction📅 Planned📊 Performance Benchmarks (v0.5.0)Test MethodologyPerformance profiles are compiled using a high-resolution, hardware-isolated tracking framework tracking nanosecond-precision execution windows. The benchmark bypasses host execution variance, measuring only the parsing, adaptation, and normalization pipelines.Environment Platform: Windows 11 Core + WSL2 (Ubuntu Linux Kernel)Compiler Stack: GCC C11 Optimization Profile (-Wall -Wextra -O3)Test Workload Scale: 100,000 Synchronous AWS CUR 2.0 Data Rows (6.54 MB payload)Telemetry Baseline ResultsMetric Performance VectorReal-World Empirical ValueData Processing Bandwidth35.49 MB/secIngestion Pipeline Velocity542,535 records/secondTotal Pipeline Runtime184.32 msAverage Latency Cost1,843.20 nanoseconds / record📁 Repository LayoutThe workspace is strictly partitioned to separate the ingestion plane from the verification engines:Plaintextbilling-data-gateway/
├── include/
│   ├── ifm_spec.h             # Canonical primitives and record structural layout
│   ├── mmap_reader.h          # Low-level POSIX paging and virtual mapping handles
│   ├── provider_adapters.h    # Hyperscaler structural interface abstractions
│   ├── provider_registry.h    # Runtime header routing definitions
│   └── pipeline.h             # E2E pipeline synchronization controls
├── src/
│   ├── mmap_reader.c          # Core virtual paging implementation
│   ├── adapters/
│   │   ├── aws_adapter.c      # AWS CUR 2.0 dynamic header mapping logic
│   │   └── azure_adapter.c    # Azure Cost Management mapping logic
│   ├── providers/
│   │   └── provider_registry.c# Auto-detection and routing matrix
│   ├── utils/
│   │   └── fixed_point.c      # High-speed micro-currency parsing routines
│   └── pipeline.c             # Main pipeline orchestration loop
├── tests/
│   └── test_pipeline.c       # End-to-end regression test suite
├── benchmarks/
│   └── bench_throughput.c     # POSIX CLOCK_MONOTONIC timing suite
└── README.md                  # Project architectural baseline documentation
⚡ Quick StartPrerequisitesCompiler: GCC (C11 support required)OS Environment: POSIX-compliant system (Linux, WSL2, macOS)1. Execute Compilation CheckCompile the system using standard optimized compiler configurations:Bashgcc -Wall -Wextra -O3 -Iinclude \
    src/mmap_reader.c \
    src/utils/fixed_point.c \
    src/adapters/aws_adapter.c \
    src/adapters/azure_adapter.c \
    src/providers/provider_registry.c \
    src/pipeline.c \
    tests/test_pipeline.c \
    -o test_runner
2. Run Integration TestsBash./test_runner
Expected Terminal Telemetry:Plaintext[*] Running Day 13 Pipeline End-to-End Verification Harness...
[+] SUCCESS: Pipeline executed, zero-copy string slices valid, micro-currency verified!
3. Run Throughput BenchmarksCompile and invoke the high-resolution performance suite:Bashgcc -Wall -Wextra -O3 -Iinclude \
    src/mmap_reader.c \
    src/utils/fixed_point.c \
    src/adapters/aws_adapter.c \
    src/adapters/azure_adapter.c \
    src/providers/provider_registry.c \
    src/pipeline.c \
    benchmarks/bench_throughput.c \
    -o bench_runner && ./bench_runner
🗺️ Product RoadmapPlaintext  [v0.5.0]  ──►  [v0.6.0]  ──►  [v0.7.0]  ──►  [v0.8.0]  ──►  [v1.0.0]
  Benchmark      CLI Binary     Embed API    Serializers    Production
  Telemetry      Gateway        Library      JSON/Parquet   Release
  (Current)      (Next Sprint)  (libbg.a)    Output Plane   Stable Core
v0.5.0 (Current Milestone): Empirical throughput performance benchmarking and high-resolution nanosecond verification tracking.v0.6.0 (Next Sprint): Implementation of user-facing Command Line Interface (getopt.h) for decoupled runtime pipeline invocation.v0.7.0: Compilation into an installable embeddable static library interface (libbillinggateway.a).v0.8.0: High-velocity output serializers (JSON, Apache Parquet, Apache Arrow format conversions).📄 LicenseThis repository is open-source infrastructure software released under the terms of the MIT License.