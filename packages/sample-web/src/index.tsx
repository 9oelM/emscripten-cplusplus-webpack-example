import React from "react"
import ReactDOM from "react-dom"
import { ExampleImpure } from "./components/Example"
// import * as loadedModule from "../../sample-wasm"
// loadedModule.then((i) => console.log(i.// let loadedModule = null
// (20)))

// const a = loadedModule

ReactDOM.render(<ExampleImpure />, document.getElementById(`root`))
