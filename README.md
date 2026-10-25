<div align="center" style="background-color: #0d1117; padding: 20px; border-radius: 8px;">

<img src="Stargif.gif" width="500" alt="Star Logo">

# Star

<p>
  <img src="https://img.shields.io/badge/License-GNU_GPL_v3-blue?style=flat-square">
  <img src="https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white">
</p>

<p><a href="https://sirmick.github.io/ProjectStar/"><strong>▶ Try Star in your browser</strong></a></p>

</div>

---

Star is a compiled programming language aiming to achieve the syntax of Python and the speed of C++. Star uses C++ to translate clean Python-like code into optimized C++, bridging Python and C++.

## Why Star?

Star, being a transpiler to C++, inherits the execution model of C++. The compiled C++ file, and the `.cpp` file itself is very portable, and acts as a "build state" for Star. Star inherits all the advantages of C++, including not just importing other Star files, but natively importing Python, C++, and Rust through pybind11 and other similar scripts.

## Example Code

Below is an example of the simple Star syntax for your reference:

```star
define greet(name) {  # This is a comment
    print(var"hello!<name>!")  var"" is the Star equivalent to Python's fstring
}
```

## Web Playground

`web/` holds a browser playground: type Star code in a text box and see the program's output and the generated C++. The compiler runs in the page as WebAssembly. To build it you need [Emscripten](https://emscripten.org):

```sh
./web/build.sh      # produces web/star.js
```

A live build is at **https://sirmick.github.io/ProjectStar/**, deployed by `.github/workflows/PAGES.yaml` on every push to `main`. To run it locally, open `web/index.html` in a browser (no server needed). `starimport`/`cppimport` aren't available in the playground, since there's no file system there.
