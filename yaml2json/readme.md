Prerequisites
-------------

* `yaml2json` is written in Haskell, and depends on the Haskell Platform.

  A cabal file is provided, which should take care of dependencies such as the
  yaml and json packages.
  Use `cabal install --only-dependencies` to install them.

Compile
-------

* To compile and run the tests use the `build.sh` script provided.

* The build script builds using cabal and tests using the `test.sh` script,
  both of which you can do independently.

Install
-------

* Use `cabal install` to install

This would of course make the executable dependent on the dynamic libs being available at runtime.
