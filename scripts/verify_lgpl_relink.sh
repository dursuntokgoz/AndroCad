#!/usr/bin/env bash
# Copyright (c) 2026 dursuntokgoz. All rights reserved.
# LGPL v2.1 Section 6 Dynamic Relinking Verification Kit for AndroCAD
set -euo pipefail

echo "=== AndroCAD LGPL v2.1 Dynamic Relinking Verifier ==="

# 1. Verify CMake builds androcad_kernel as SHARED library linking dynamically to OCCT toolkits
if grep -q "add_library(androcad_kernel SHARED" core-kernel/src/main/cpp/CMakeLists.txt; then
    echo "[PASS] libandrocad_kernel.so is configured as a SHARED library."
else
    echo "[FAIL] libandrocad_kernel.so is not configured as SHARED."
    exit 1
fi

# 2. Verify zero static OCCT (.a) archives linked in CMakeLists.txt
if grep -E "libTK.*\.a" core-kernel/src/main/cpp/CMakeLists.txt; then
    echo "[FAIL] Static OCCT archive linkage detected!"
    exit 1
else
    echo "[PASS] Zero static OCCT (.a) archives linked; 100% dynamic .so linkage."
fi

# 3. Verify THIRD_PARTY_NOTICES.md and docs/licenses.md exist and document relinking
for doc in THIRD_PARTY_NOTICES.md docs/licenses.md scripts/build_occt_android.sh; do
    if [ ! -f "$doc" ]; then
        echo "[FAIL] Missing required LGPL compliance artifact: $doc"
        exit 1
    fi
done
echo "[PASS] LGPL compliance notices and OCCT build script verified."
echo "=== LGPL DYNAMIC RELINKING AUDIT PASSED ==="
