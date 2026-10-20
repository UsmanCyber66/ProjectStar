<div align="center">
  <picture>
<video src="Starlogo.mp4" autoplay muted playsinline width="100%"></video>
  </picture>
Star
#
<img src="https://img.shields.io/badge/License-GNU_GPL_v3-blue" width=180>
<img src="https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white" width=180>
</div>
# ***Star***
Star is compiled programming language aiming to achieve the syntax of Python and Speed of C++. Star uses C++ to translate clean Python-like code into optimized C++, bridging Python and C++.

## Why Star?
Star , being a transpiler to C++, inherits the execution model of C++. The compiled C++ file, and the .CPP file itself is very portable, and acts as a "build state" for Star. Star inherits all the advantages of C++, including not just importing other Star files into python, but natively importing Python, C++, and Rust through pybind11 and other similar scripts.

Below is an example of the simple Star syntax for your reference.
## Example Code
```StarCode
define greet(name){  #This is a comment
printvar("hello!<name>!") #Printvar is the Star equivalent to Python's fstring
}
```

