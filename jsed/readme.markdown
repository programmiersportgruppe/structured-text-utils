# Cookbook

## Basic Usage

`jsed` applies javascript functions to json objects and returns
the result as json. Here a basic example:

~~~ {.bash}
echo '{"firstName": "Bart", "lastName": "Simpson"}' | jsed 'function(x) ({name: x.firstName + " " + x.lastName})'
~~~

yields:

~~~ {.json}
{"name":"Bar Simpson"}
~~~

There is also support for pretty-printed json using the `--pretty` option.

~~~ {.bash}
echo '{"firstName": "Bart", "lastName": "Simpson"}' \
| jsed --pretty 'function(x) ({name: x.firstName + " " + x.lastName})'
~~~

yields:

~~~ {.json}
{
    "name": "Bart Simpson"
}
~~~

The transformation function can also be read from a file using the `-f` parameter.
This comes handy when transformations are more complicated.

~~~ {.bash}
echo 'function(x) ({name: x.firstName + " " + x.lastName})'>fun.js
echo '{"firstName": "Bart", "lastName": "Simpson"}' | jsed -f fun.js
~~~



## Raw Output

If the transformation function returns a string, the json serialisation can be
bypassed using the `--raw` option.

~~~ {.bash}
echo '{"firstName": "Bart", "lastName": "Simpson"}' | jsed --raw 'function(x) x.firstName + " " + x.lastName'
~~~

yields:

~~~
Bart Simpson
~~~



## Multi Document Stream Support

The multi document mode supports applying the function to streams
of newline separated JSON documents. It is activated with the
`--multi-docs` option:

~~~~ .bash
jsed --multi-docs 'function(x) ({name: x.firstName})' << END
{"firstName":"Bart","lastName":"Simpson"}
{"firstName":"Lisa","lastName":"Simpson"}
END
~~~~

yields:

~~~
{"name":"Bart"}
{"name":"Lisa"}
~~~

## Javascript Helpers

`jsed` does provide methods that can be used in transformations.

### `filter` objects/ hashes

The `filter` method for `object`s works similar
to the `filter` method for `array`s. It takes a predicate that
has two parameters the key and the value for each element.

In the following example pairs get filtered depending on the value
component:

~~~ {.bash}
echo '{ "name": "felix", "occupation": "developer", "test":"hello"}' \
| jsed 'function(x) x.filter(function(k, v) v != "hello")'
~~~

yields:

~~~
{"name":"felix","occupation":"developer"}
~~~

### `map` objects/ hashes

The `map` method on `object` works simlar to
the `map` method on `array`. A transformation function
gets applied to each key value pair, and returns the new value


~~~ {.bash}
echo '{ "name": "felix", "occupation": "developer"}'\
| jsed 'function(x) x.map(function(k, v) v.toUpperCase())'
~~~

yields:

~~~ {.json}
{"name":"FELIX","occupation":"DEVELOPER"}
~~~

### `pipe` strings

The `pipe` method allows to pipe/ filter strings through an external utility.
It takes two arguments, the name of the utility and a an array containing the command
line parameters.

~~~ {.bash}
echo '{"value":"hello\ngoodbye"}'|jsed --raw 'function(x) x.value.pipe("grep", ["hel"])'
~~~

yields:

~~~
hello
~~~

## Getting Help

`jsed` tries to be a good citizen in the unix world and
hence provides a basic command line reference when
invoked with the `--help` parameter:

~~~ {.bash}
jsed --help
~~~

yields:

~~~
Usage: jsed [options] [--in-place filename] (transformation | -f file)
   transformation   Transformation function
   file             File containing transformation function
Options:
   -m, --multi-docs Expects input to be multiple documents
                    formatted on a single line and separated
                    by new lines
   -r, --raw        Produces raw string output
   -p, --pretty     Pretty print json output
   -h, --help       Display help message
   -d, --debug      Print debug information
~~~


# Compiling jsed

* `jsed` depends on the spidermonkey library being available on your machine.

*  The `fetch-dependencies.sh` script can be used to download, compile and install
   spidermonkey (watch out there is a `sudo make install` in there).

* To compile and run basic tests use the `build.sh` script provided. Please note that potentially
  you might have to fix the include and the lib path.

* The `install.sh` script can be used to install jsed



