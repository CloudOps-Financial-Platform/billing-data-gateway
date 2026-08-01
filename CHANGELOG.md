# Changelog

All notable changes to the **Billing Data Gateway** software asset will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-08-01

### Added
- **System 5 Validation Engine:** Invariant rule evaluation for raw vendor inputs (`RECORD_MISSING_KEY`, `RECORD_INVALID_DATE`, `RECORD_EMPTY_RESOURCE`).
- **Validation Bitmask Telemetry:** Exposed `flags` bitmask in serialized output streams for full pipeline observability.
- **System 6 Canonical CSV Exporter:** Support for `-f csv` alongside `-f json` streaming output formats.
- **AWS Header Alias Resolution:** Extended `aws_adapter.c` to resolve both `lineItem/UsageAccountId` and `lineItem/AccountId`.
- **System 8 Validation Assertions:** Extended automated regression suite (`tests/test_pipeline.c`) with bitmask assertions.
- **High-Resolution Benchmark Harness:** Updated `benchmarks/bench_throughput.c` for high-volume stress testing over mapped memory pages.

### Changed
- **Memory Ownership Model:** Shifted POSIX `mmap()` handle lifecycle to `main()`, guaranteeing $O(1)$ zero-copy string slice validity throughout serialization.
- **CLI Options:** Expanded POSIX `getopt()` flags to support `-f json` and `-f csv` output format dispatching.

### Fixed
- **Upstream Column Mapping Defect:** Fixed header resolution failure where `lineItem/AccountId` failed to map to `account_id_idx`.
- **Validation Bitmask Reporting:** Fixed serialization output to explicitly render `r->flags` bitmask integer.

---
*Engineered by Naresh (CN2 Systems Co.) — AI-Native Quant Systems Architect.*