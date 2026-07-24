#!/bin/bash

# CloudOps Financial Platform — Automated Test Suite
# Returns 0 if all assertions pass, 1 if any regression is detected.

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

PASSED=0
FAILED=0

run_test() {
    local name="$1"
    local command="$2"
    local expected_exit="$3"

    eval "$command" > /dev/null 2>&1
    local exit_code=$?

    if [ $exit_code -eq$expected_exit ]; then
        echo -e "[ ${GREEN}PASS${NC} ] $name (Exit Code:$exit_code)"
        PASSED=$((PASSED + 1))
    else
        echo -e "[ ${RED}FAIL${NC} ]$name (Expected: $expected_exit, Got:$exit_code)"
        FAILED=$((FAILED + 1))
    fi
}

echo "================================================================="
echo "   CLOUDOPS BILLING DATA GATEWAY — REGRESSION SUITE            "
echo "================================================================="

# 1. Test Valid Shifted Schema (Must Exit 0)
run_test "AWS CUR Shifted Schema Ingestion" "./billing_gateway data/aws_cur_shifted.csv" 0

# 2. Test Missing Required Provider Header (Must Hard-Abort, Exit 1)
run_test "Missing Provider Header Hard-Abort" "./billing_gateway data/err_missing_provider.csv" 1

# 3. Test Duplicate Header Overwrite (Must Handle Gracefully, Exit 0)
run_test "Duplicate Header Resilience" "./billing_gateway data/err_duplicate_header.csv" 0

# 4. Test Extra Unmapped Columns (Must Skip Unmapped Fields, Exit 0)
run_test "Unknown Metadata Columns Filtering" "./billing_gateway data/err_unknown_columns.csv" 0

echo "================================================================="
echo -e " Summary: ${GREEN}$PASSED Passed${NC} \vert{}${RED}$FAILED Failed${NC}"
echo "================================================================="

if [ $FAILED -ne 0 ]; then
    exit 1
fi
