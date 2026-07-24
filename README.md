# CloudOps Financial Platform — Billing Data Gateway

> Enterprise-grade, zero-copy multi-cloud cost billing ingestion engine engineered in C11 POSIX infrastructure.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-v0.3.2-blue)
![Language](https://img.shields.io/badge/language-C11-blue)
![Memory Model](https://img.shields.io/badge/memory--model-POSIX%20mmap()-orange)
![Test Suite](https://img.shields.io/badge/tests-4%2F4%20passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 🎯 System Architecture & Ingestion Flow

The `billing-data-gateway` is a high-performance C systems module designed to stream, normalize, and validate raw cloud billing exports (AWS CUR 2.0, Azure Cost Management, GCP Billing) directly into memory using zero-copy POSIX virtual memory mapping (`mmap`).

```text
  Raw Cloud Billing Export (AWS CUR / Azure / GCP CSV)
                          │
                          ▼
        [ POSIX mmap() Virtual File Pager ]
                          │
                          ▼
      [ Dynamic Header Resolver & Alias Matrix ]
                          │
         ┌────────────────┴────────────────┐
         ▼                                 ▼
  [ FAIL: Missing Anchors ]        [ PASS: Header Offset Map ]
         │                                 │
         ▼                                 ▼
  [ Hard-Abort Ingestion Gate ]    [ Zero-Heap Pointer Tokenizer ]
  (0 Corrupt Memory Mutations)             │
                                           ▼
                                [ In-Memory IFM Binary Records ]
                                           │
                                           ▼
                                [ Out-Of-Band Line Quarantine ]
🛡️ Core Architectural InvariantsZero-Copy Virtual Memory Paging (mmap): Bypasses kernel-to-userland buffer copying by mapping file descriptors directly into virtual memory address space.Zero-Heap Allocation Tokenization (str_slice_t): Operates entirely on string pointer slices without executing runtime malloc calls inside streaming loops.Data-Driven Dynamic Schema Resolution: Uses an aliased lookup matrix (header_map_t) to dynamically resolve column offsets out-of-order, handling vendor schema drift seamlessly.Hard-Abort Defensive Validation Gate: Enforces structural integrity before row streaming; missing required financial anchors (provider, billed_cost) immediately trigger a clean, fatal abort.Out-of-Band Fault Containment: Isolates malformed or corrupted row inputs into dedicated error quarantine channels without terminating the pipeline.⚡ Quickstart & CompilationPrerequisitesCompiler: GCC (C11 support required, -Wall -Wextra -O3 -march=native)Environment: POSIX compliant OS (Linux, WSL2, macOS)Build Tool: GNU Make1. Build the BinaryBash# Clone the repository
git clone [https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git](https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git)
cd billing-data-gateway

# Clean and compile optimized binary
make clean && make
2. Execute Automated Regression SuiteBash# Run automated test harness verifying 4 threat scenarios
make test
3. Run Gateway Against Custom Shifted DatasetsBash# Ingest shifted AWS CUR 2.0 dataset
./billing_gateway data/aws_cur_shifted.csv
📊 Ingestion Benchmark & Reliability MetricsDataset TypeTarget SchemaTotal RowsIngestion LatencyDataset Pass RateSystem StatusAWS CUR 2.0 ShiftedOut-of-Order Columns30.0166 ms100.00%PASSEDMissing Provider HeaderMissing Required Key1< 0.0050 ms0.00%HARD-ABORT (Pass)Duplicate Header LineOverwritten Schema10.0120 ms100.00%PASSED (Warning Logged)Unknown Extra ColumnsMetadata Pollution10.0110 ms100.00%PASSED (Columns Skipped)📁 Directory StructurePlaintextbilling-data-gateway/
├── include/
│   ├── csv_parser.h         # Zero-copy pointer tokenizer & slice structures
│   ├── ifm_core.h           # Sovereign Intermediate Financial Model (IFM) specs
│   ├── mmap_reader.h        # POSIX mmap() virtual file pager interface
│   └── provider_adapters.h  # Dynamic header map & multi-cloud alias definitions
├── src/
│   ├── csv_parser.c         # Fast line tokenizer & safe numeric converters
│   ├── main.c               # Entrypoint, header validator & row streaming loop
│   ├── mmap_reader.c        # Low-level POSIX mmap management
│   └── provider_adapters.c  # Data-driven alias matrix & row normalizer
├── data/
│   ├── aws_cur_shifted.csv       # Shifted AWS CUR 2.0 test dataset
│   ├── err_duplicate_header.csv  # Duplicate column validation dataset
│   ├── err_missing_provider.csv  # Missing anchor validation dataset
│   └── err_unknown_columns.csv   # Extra metadata column test dataset
├── tests/
│   └── run_tests.sh         # Automated regression test harness
├── Makefile                 # Optimized C11 build & test automation script
├── LICENSE                  # MIT License
└── README.md                # System architectural documentation
🗺️ Milestone Roadmap[x] v0.1.0 — POSIX mmap() virtual file pager & zero-copy tokenizer.[x] v0.2.0 — Defensive numeric conversions & out-of-band row quarantine logging.[x] v0.3.2 — Dynamic schema resolution, hard-abort validation gate & automated test harness (make test).[ ] v0.4.0 — Multi-cloud schema expansion (Full Azure Cost Management & GCP Billing aliases).[ ] v0.5.0 — SIMD-accelerated newline scanning & multi-threaded batch ingestion.📜 LicenseThis project is open-source infrastructure software released under the MIT License.
