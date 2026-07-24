# CloudOps Financial Platform — Billing Data Gateway

> High-performance C11 billing ingestion engine utilizing zero-copy parsing, dynamic schema normalization, and defensive memory constraints.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Version](https://img.shields.io/badge/version-v0.3.2-blue)
![Language](https://img.shields.io/badge/language-C11-blue)
![Memory Model](https://img.shields.io/badge/memory--model-POSIX%20mmap()-orange)
![Test Suite](https://img.shields.io/badge/tests-4%2F4%20passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 🎯 Architectural Ingestion Flow

The `billing-data-gateway` streams, normalizes, and validates raw cloud billing exports into memory using zero-copy POSIX virtual memory mapping (`mmap`).

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
  (Execution Terminated)                   │
                                           ▼
                                [ Zero-Heap Pointer Tokenizer ]
                                           │
                                           ▼
                                [ In-Memory IFM Binary Records ]
                                           │
                                           ▼
                                [ Out-Of-Band Line Quarantine ]
💡 What is the IFM (Intermediate Financial Model)?Cloud providers export cost billing data using radically different schema structures (lineItem/UnblendedCost in AWS vs CostInBillingCurrency in Azure).The Intermediate Financial Model (IFM) is the gateway's sovereign internal data representation (ifm_record_t). It converts heterogenous vendor formats into a single, unified financial layout containing standard metrics (provider, account_id, service_name, region, billed_cost, usage_quantity).Every downstream core financial calculator operates exclusively on normalized IFM arrays.🛠️ Core Engineering PrinciplesZero-Copy Memory Mapping (mmap): Bypasses standard kernel-to-userland buffer copies by mapping file descriptors directly into virtual memory addresses.Zero-Heap Tokenization (str_slice_t): Operates on string pointer slices without allocating runtime heap memory (malloc) inside the primary data streaming loops.Dynamic Schema Resolution: Employs a data-driven alias lookup matrix to map column target offsets out-of-order, preventing ingestion failures when cloud providers change column layouts.Defensive Header Validation: Validates schemas at the file boundary. Missing vital fields (like provider or billed_cost) triggers an immediate pipeline abort to prevent processing corrupt rows.Automated Quality Assurances: All core compilation and parsing invariants are protected by a reproducible local regression test harness (make test).🌐 Cloud Provider Support MatrixCurrent Active Support (v0.3.2):✅ AWS CUR 2.0 — Fully supported with dynamic column offset mapping, case-insensitive header matching, and numeric validation loops.Planned Provider Support (v0.4.0):🟡 Azure Cost Management — Alias map defined; parser parity testing in progress.🟡 GCP Billing Export — Alias map defined; schema validation in progress.⚡ Quickstart & Automated TestingPrerequisitesCompiler: GCC (C11 support required, -Wall -Wextra -O3 -march=native)Environment: POSIX compliant OS (Linux, WSL2, macOS)Build Tool: GNU Make1. Build the BinaryBashmake clean && make
2. Run Automated Regression SuiteBashmake test
Expected Test Harness Output:Plaintext=================================================================
   CLOUDOPS BILLING DATA GATEWAY — REGRESSION SUITE            
=================================================================
[ PASS ] AWS CUR Shifted Schema Ingestion (Exit Code: 0)
[ PASS ] Missing Provider Header Hard-Abort (Exit Code: 1)
[ PASS ] Duplicate Header Resilience (Exit Code: 0)
[ PASS ] Unknown Metadata Columns Filtering (Exit Code: 0)
=================================================================
 Summary: 4 Passed | 0 Failed
=================================================================
🧪 Dataset Validation Test LogTest ScenarioTarget Input MatrixApplied System ConstraintPipeline StatusShifted AWS Schemaaws_cur_shifted.csvResolved 9 column offsets out-of-orderPASS (100% Ingested)Missing Provider Headererr_missing_provider.csvIntercepted at header gate; exited safelyPASS (Fatal Exit 1)Duplicate Header Lineerr_duplicate_header.csvLogged dynamic warning; bypassed overwritePASS (Warning Logged)Unknown Extra Columnserr_unknown_columns.csvFiltered out unmapped metadata attributesPASS (Columns Bypassed)📁 Repository StructurePlaintextbilling-data-gateway/
├── include/
│   ├── csv_parser.h         # Zero-copy tokenizer structures
│   ├── ifm_core.h           # Intermediate Financial Model primitives
│   ├── mmap_reader.h        # POSIX mmap() file management
│   └── provider_adapters.h  # Dynamic column alias mapping matrices
├── src/
│   ├── csv_parser.c         # Fast string parsers & numeric converters
│   ├── main.c               # Stream loop orchestration & header validation
│   ├── mmap_reader.c        # Low-level POSIX paging layer
│   └── provider_adapters.c  # Dynamic array resolver routines
├── data/
│   ├── aws_cur_shifted.csv       # Shifted AWS CUR test file
│   ├── err_duplicate_header.csv  # Duplicate column validation test file
│   ├── err_missing_provider.csv  # Missing anchor validation test file
│   └── err_unknown_columns.csv   # Extra metadata attribute test file
├── tests/
│   └── run_tests.sh         # Automated bash test suite
├── Makefile                 # Optimized C11 build & validation rules
├── LICENSE                  # MIT License
└── README.md                # System documentation layout
📜 Release Historyv0.3.2 (Current Release) — Dynamic header alias mapping matrix, hard-abort header validation boundaries, and automated regression test suite integration (make test).v0.2.0 — Zero-heap token extraction, numeric double/uint64 validation, and diagnostic line quarantine logging.v0.1.0 — POSIX memory-mapped file pager implementation and core engine layout.🗺️ Future Roadmap (v0.4.0 Scope)Expand alias handling matrices for full Azure Cost Management exports.Expand alias handling matrices for full GCP Billing BigQuery exports.Implement cross-provider normalization parity tests.Execute ingestion scaling benchmarks on multi-gigabyte billing files.📄 LicenseThis project is open-source infrastructure software released under the MIT License.