import { fib } from "./fib.js"
import fibonacciModule from "./fib.wasm"

// Since webpack will change the name and potentially the path of the
// `.wasm` file, we have to provide a `locateFile()` hook to redirect
// to the appropriate URL.
// More details: https://kripken.github.io/emscripten-site/docs/api_reference/module.html
const wasm = fib({
  locateFile(path) {
    if (path.endsWith(`.wasm`)) {
      return fibonacciModule
    }
    return path
  },
})

export default wasm
