# CloudOps Financial Platform — Billing Data Gateway

> High-performance C11 billing ingestion engine exploring zero-copy parsing, dynamic multi-cloud schema normalization, and defensive memory isolation.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-v0.3.2-blue)
![Language](https://img.shields.io/badge/language-C11-blue)
![Memory Model](https://img.shields.io/badge/memory--model-POSIX%20mmap()-orange)
![Test Suite](https://img.shields.io/badge/tests-4%2F4%20passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 🎯 Architectural Overview

The `billing-data-gateway` is a low-latency systems module written in C11. It streams, normalizes, and validates raw cloud billing exports directly into memory using zero-copy POSIX virtual memory mapping (`mmap`).

```text
  Raw Cloud Billing Export CSV (AWS CUR / Azure / GCP)
                          │
                          ▼
        [ POSIX mmap() Virtual File Pager ]
                          │
                          ▼
      [ Dynamic Header Resolver & Alias Matrix ]
                          │
         ┌────────────────┴────────────────┐
         ▼                                 ▼
  [ REJECT: Invalid Schema ]       [ PASS: Header Offset Map ]
  (0 Corrupt Memory Mutations)             │
                                           ▼
                                [ Zero-Heap Pointer Tokenizer ]
                                           │
                                           ▼
                                [ In-Memory IFM Binary Records ]
                                           │
                                           ▼
                                [ Out-Of-Band Line Quarantine ]
💡 What is the IFM (Intermediate Financial Model)?
Cloud providers export cost billing data using radically different schema structures (lineItem/UnblendedCost in AWS CUR 2.0 vs CostInBillingCurrency in Azure Cost Management).

The Intermediate Financial Model (IFM) is our sovereign, in-memory C data structure (ifm_record_t). It converts heterogenous vendor exports into a unified financial primitive containing standard fields (provider, account_id, service_name, region, billed_cost, usage_quantity). Every downstream billing calculator operates exclusively on IFM records.
🛠️ Core Engineering Principles
Zero-Copy Memory Mapping (mmap): Bypasses kernel-to-userland buffer copying by mapping file descriptors directly into virtual memory address space.

Zero-Heap Tokenization (str_slice_t): Operates entirely on string pointer slices without executing runtime malloc calls inside streaming loops.

Data-Driven Dynamic Schema Resolution: Uses an aliased lookup matrix (header_map_t) to dynamically resolve column offsets out-of-order, handling vendor schema drift seamlessly.

Defensive Header Validation Gate: Enforces structural integrity before row streaming; missing required financial anchors (provider, billed_cost) immediately trigger a clean, fatal abort.

Reproducible Quality Controls: All pipeline invariants are enforced through an automated bash regression test harness (make test).
🌐 Cloud Provider Support Matrix
Current Active Support (v0.3.2):

✅ AWS CUR 2.0 — Fully supported with dynamic column offset mapping, case-insensitive header matching, and safe numeric type parsing.

Planned Provider Support (v0.4.0):

🟡 Azure Cost Management — Alias map defined; provider parity verification in progress.

🟡 GCP Billing Export — Alias map defined; BigQuery CSV export testing in progress.

⚡ Quickstart & Automated Testing
Prerequisites
Compiler: GCC (C11 support required, -Wall -Wextra -O3 -march=native)

Environment: POSIX compliant OS (Linux, WSL2, macOS)

Build Tool: GNU Make
1. Build the Binary
# Clone the repository
git clone [https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git](https://github.com/CloudOps-Financial-Platform/billing-data-gateway.git)
cd billing-data-gateway

# Clean and compile optimized binary
make clean && make
2. Run Automated Regression Suite
make test
Expected Test Harness Output:
=================================================================
   CLOUDOPS BILLING DATA GATEWAY — REGRESSION SUITE            
=================================================================
[ PASS ] AWS CUR Shifted Schema Ingestion (Exit Code: 0)
[ PASS ] Missing Provider Header Hard-Abort (Exit Code: 1)
[ PASS ] Duplicate Header Resilience (Exit Code: 0)
[ PASS ] Unknown Metadata Columns Filtering (Exit Code: 0)
=================================================================
 Summary: 4 Passed | 0 Failed
=================================================================
🧪 Dataset Validation Suite Results
Test Scenario               Target Input Matrix          System Action                                   Pipeline Status
Shifted AWS Schema          aws_cur_shifted.csv          Resolved 9 column offsets out-of-order          PASS (100% Ingested)
Missing Provider Header     err_missing_provider.csv     Triggered hard-abort gate; 0 rows touched       PASS (Fatal Exit 1)
Duplicate Header Line       err_duplicate_header.csv     Overwrote duplicate; logged warning             PASS (Warning Logged)
Unknown Extra Columns       err_unknown_columns.csv      Skipped unmapped metadata columns               PASS (Columns Filtered)
📁 Repository Structure
billing-data-gateway/
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
📜 Release History
v0.3.2 (Current) — Integrated Dynamic Header Mapping, Hard-Abort Header Validation Gate, Data-Driven Alias Matrix, and Automated Regression Test Suite (make test).

v0.2.0 — Zero-heap pointer tokenizer, defensive numeric string parsing, and out-of-band row quarantine logging.

v0.1.0 — Initial POSIX mmap() virtual file pager and core project build layout.
📄 License
This project is open-source infrastructure software released under the MIT License.
