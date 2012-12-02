function(input, transformation, raw){

    var inputJson = JSON.parse(input)
    var result = transformation(inputJson)
    var output = raw ? result : JSON.stringify(result)
    //todo guard against output not being a string
    return output
}

