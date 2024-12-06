# cppstarter
A C++ starter template for a header only library

## How to use?
This is a header only library, so it does not require additional steps, just copy `cppstarter.h` from `include/` directory and use it in your project.

## Running tests
Install Meson, and a backend (such as Ninja) and run the following commands:

```
$ meson wrap install gtest
```

If you have clang, along with sanitizers installed
```
$ CXX=clang++ meson setup -Db_sanitize=address -Ddevelopment=true -Denable-tests=true -Db_lundef=false --reconfigure builddir
```

Otherwise
```
$ meson setup -Denable-tests=true --reconfigure builddir
```

```
$ cd builddir
$ meson test -v
```