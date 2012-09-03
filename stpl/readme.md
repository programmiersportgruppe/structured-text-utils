Prerequisites
-------------

* `stpl` is written in Haskell, and depends on the Haskell Platform.

  A cabal file is provided, which should take care of dependencies such as the
  hStringTemplate and json packages.
  Use `cabal install --only-dependencies` to install them.

Compile
-------

* To compile and run the tests use the `build.sh` script provided.

* The build script builds using cabal and tests using the `test.sh` script,
  both of which you can do independently.

Install
-------

* Use `cabal install` to install

Executable Size
---------------

By default, Haskell links everything statically, producing huge binaries.
I just got a 7.7M `stpl` binary that `strip`s to 4.5M.

It should be possible to get much smaller binaries (tens of kilobytes)
if everything (including dependencies) is compiled dynamically.
This would of course make the executable dependent on the dynamic libs being available at runtime.
