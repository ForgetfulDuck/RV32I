for file in *.sv; do
  ./Verilatte.sh "${file%.sv}"
done