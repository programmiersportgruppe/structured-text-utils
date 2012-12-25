#ifndef JS_H_INCLUDED
#define JS_H_INCLUDED
#include <string>
#include "jsapi.h"

class Function;

class ContextWrapper;

class JSInterpreter {
    private:

    JSRuntime *rt;
    JSContext *cx;
    JSObject  *global;
    ContextWrapper *cw;


    public:

    JSInterpreter();
    JSInterpreter(const JSInterpreter&);
    Function evaluateScript(std::string script);

    std::string invoke(const Function function, std::string input, Function transformation, bool raw, bool pretty);

    ~JSInterpreter();
};



class Function {
    private:
    JSInterpreter& interpreter;
    jsval jsFunction;

    public:
    Function(JSInterpreter&, jsval);
    operator jsval() const;
    std::string invoke(std::string input, Function transformation, bool raw, bool pretty) const;
};

#endif

