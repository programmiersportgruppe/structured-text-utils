structured-text-utils
=====================

A group of utilities to deal with structured text such as json or yaml.

jsed
----

A utility to filter json using a javascript function in the spirit of `sed` and `awk`.

This is best illustrated with an example:

~~~ .bash
echo '{"firstName": "Bart", "lastName": "Simpson"}' | jsed 'function(x) x.firstName + " " + x.lastName'
~~~

yields:

~~~
"Bart Simpson"
~~~

(Note the double quotes, the result is being rendered as JSON)

The javascript version used is 1.85, so we can use the nice expression syntax that is shown above.


