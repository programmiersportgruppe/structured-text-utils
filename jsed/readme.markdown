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

## Pretty Output

There is also support for pretty-printed json using the `--pretty` option.

~~~ {.bash}
echo '{"firstName": "Bart", "lastName": "Simpson"}' | jsed --pretty 'function(x) ({name: x.firstName + " " + x.lastName})'
~~~

yields:

~~~ {.json}
{
    "name": "Bart Simpson"
}
~~~

## Multi-Document Stream Support

The multi document mode supports applying the function to streams
of newline separated JSON documents. It is activated with the
`--multi-docs` option:

~~~~ .bash
jsed --multi-docs 'function(x) x.firstName' << END
{"firstName":"Bart","lastName":"Simpson"}
{"firstName":"Lisa","lastName":"Simpson"}
END
~~~~

yields:

~~~
"Bart"
"Lisa"
~~~


# Compiling jsed

* `jsed` depends on the spidermonkey library being available on your machine.

*  The `fetch-dependencies.sh` script can be used to download, compile and install
   spidermonkey (watch out there is a `sudo make install` in there).

* To compile and run basic tests use the `build.sh` script provided. Please note that potentially
  you might have to fix the include and the lib path.

* The `install.sh` script can be used to install jsed



