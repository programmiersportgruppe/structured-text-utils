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

