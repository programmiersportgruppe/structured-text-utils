Object.prototype.map = function(mapping) {
    var ret = {};
    for (var property in this) {
        if (this.hasOwnProperty(property)) {
            ret[property] = mapping(property, this[property]);
        }
    }
    return ret;
};

Object.prototype.filter = function(predicate) {
    var ret = {};
    for (var property in this) {
        if (this.hasOwnProperty(property) && predicate(property, this[property]) ) {
            ret[property] = this[property];
        }
    }
    return ret;
};

String.prototype.pipe = function(command, args){
    return filterWrapper(this, command, args);
}


function toString(x) {
    return x.toString();
}

/** Wraps the native function and does validation */
function filterWrapper(input, command, args){
    //todo: do some guarding here (signature string, string, array<string>))
    //
    return _native_filter(input, command, ["x"]);
}


function(input, transformation, raw, pretty){

    var inputJson = JSON.parse(input)
    var result = transformation(inputJson)
    var output = raw ? result
                     : ( pretty ?
                           JSON.stringify(result, null, "    ")
                         : JSON.stringify(result)
                        )
    //todo guard against output not being a string
    return output
}

