# risc-v-web-simulator


## Compiling Kite to Wasm
Use docker to do so.

```bash
docker build ./packages/kite -t kite-wasm

docker run -v $(pwd)/packages/kite:/etc/kite-wasm kite-wasm
```

## References
- https://github.com/PetterS/clang-wasm
- https://towardsdev.com/lets-export-the-c-library-to-javascript-wasm-library-part-a-4b99c9a7cf2