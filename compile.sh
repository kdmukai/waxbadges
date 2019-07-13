eosio-cpp -o waxbadges.wasm waxbadges.cpp --abigen
mkdir -p build
cp waxbadges.wasm build/.
cp waxbadges.abi build/.
