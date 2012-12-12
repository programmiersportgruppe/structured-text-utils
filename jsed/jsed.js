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

