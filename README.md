<div align="center">

<img src="Stargif.gif" width="240" alt="Star Logo">

# Star

<p>
  <img src="https://img.shields.io/badge/License-GNU_GPL_v3-blue?style=flat-square">
  <img src="https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white">
</p>

</div>

---

Star is a compiled programming language aiming to achieve the syntax of Python and the speed of C++. Star uses C++ to translate clean Python-like code into optimized C++, bridging Python and C++.

## Why Star?

Star, being a transpiler to C++, inherits the execution model of C++. The compiled C++ file, and the `.cpp` file itself is very portable, and acts as a "build state" for Star. Star inherits all the advantages of C++, including not just importing other Star files, but natively importing Python, C++, and Rust through pybind11 and other similar scripts.

## Example Code

Below is an example of the simple Star syntax for your reference:

```star
define greet(name) {  # This is a comment
    printvar("hello!<name>!")  # Printvar is the Star equivalent to Python's fstring
}
