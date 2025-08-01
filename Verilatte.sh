#!/usr/bin/env sh
set -e

if [ -z "$1" ]; then
    echo "❌ Missing test component: $0 <test_component>"
    exit 1
fi

TEST="$1"

echo "🔧 Verilating $TEST..."
verilator -I./src -f verilator.f ./src/${TEST}.sv tb/${TEST}_tb.cpp

echo "🛠️  Compiling C++ simulation..."
make -C obj_dir -f V${TEST}.mk V${TEST}

echo "🚀 Running simulation..."
./obj_dir/V${TEST}
