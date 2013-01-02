# structured-text-utils

A group of utilities to deal with structured text such as json or yaml.


## jsed


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

There is also a multi document mode that supports applying the function to multiple JSON documents that are
rendered into single lines separated by newline characters:

~~~~ .bash
jsed -m 'function(x) x.firstName' << END
{"firstName":"Bart","lastName":"Simpson"}
{"firstName":"Lisa","lastName":"Simpson"}
END
~~~~

will yield:

~~~
"Bart"
"Lisa"
~~~

## stpl

A utility to instantiate StringTemplate templates with JSON data.

For example:

~~~ .bash
cat >report_card.st <<'END'
Student: $firstName$ $lastName$
Grades: $grades;separator=", "$
END

echo '{"firstName": "Bart", "lastName": "Simpson", "grades": ["F", "F", "F"]}' | stpl -t report_card.st
~~~

yields:

~~~
Student: Bart Simpson
Grades: F, F, F
~~~


## yaml2json

A utitility to convert YAML to JSON

Example:

~~~ .bash
yaml2json <<END
firstName: Bart
lastName: Simplson
grades:
    - F
    - F
    - F
END
~~~

will yield:

~~~ .json
{"firstName": "Bart", "lastName": "Simpson", "grades": ["F", "F", "F"]}
~~~


## sqljson

A utility to run sql queries to return json. Not yet implemented.

~~~~
t
x   y
x1 y1
x2 y2
~~~~

~~~
select x,y from t
~~~~

Variant I
~~~~
[{"x": "x1", "y":"y1"},{"x":"x2", "y":"y2"}]
~~~~

Variant II
~~~
{"x": "x1", "y":"y1"}
{"x":"x2", "y":"y2"}
~~~

### Joining

~~~
select  t1.x, t1.y, t2.z from t1 left join t2 on t1.y = t2.y
~~~

SQL result:
~~~
(x1 y1) z1
x1 y1 z2
x1 y1 z3
x2 y2 z9
x2 y2 z10
~~~

But what you want:

~~~
{ "x":"x1", "y":"y1", "t2":[ {"z":"z1"}, {"z":"z2"}, {"z","z3"}]}
{ "x":"x2", "y":"y2", "t2":[ {"z":"z9"}, {"z":"z10"}]}
~~~








