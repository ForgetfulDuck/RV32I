#!/usr/bin/env bash
set -e

# Read arguments into an array of modules to skip
# This syntax requires bash, hence the shebang change
skip_modules=("$@")

echo "🏁 Starting batch verification of all SV modules"
echo "========================================"
# Display skipped modules, handling the case where none are skipped
if [ ${#skip_modules[@]} -eq 0 ]; then
    echo "Skipping no modules."
else
    echo "Skipping modules: ${skip_modules[*]}"
fi
echo "========================================\n"


for file in src/*.sv; do
    module=$(basename "$file" .sv)
    
    should_skip=0 # Flag to indicate if the current module should be skipped
    for skip_module in "${skip_modules[@]}"; do
        if [ "$module" = "$skip_module" ]; then
            echo "⏭️ Skipping module: $module (as requested)"
            should_skip=1
            break # Exit the inner loop as we found a match
        fi
    done

    if [ "$should_skip" -eq 1 ]; then
        echo "========================================"
        continue
    fi

    echo "🔍 Processing module: $module"
    echo "------------------------------"
    ./Verilatte.sh "$module"
    echo "========================================"
done

echo "✅ All non-skipped modules verified"