#!/usr/bin/env ruby


def decorate_line(js_line)
    return  '"' + js_line.gsub(/\\/,"\\"*4).gsub(/\"/,"\\\"") + '\\n"'
end

lines = STDIN.read.split("\n").map{|line| decorate_line(line)}

puts "/* This file is generated */\n"
puts "const char *jsSource = " + lines[0]

lines = lines[1..-1].map{|line| " "*23 + line}

puts lines.join("\n") + ";"


