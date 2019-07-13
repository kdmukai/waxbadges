eosio-cpp -o waxbadge.wasm waxbadge.cpp --abigen
mkdir -p build
cp waxbadge.wasm build/.
cp waxbadge.abi build/.
