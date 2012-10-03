function(input, transformation){
    var inputJson = JSON.parse(input)
    return JSON.stringify(transformation(inputJson))
}

