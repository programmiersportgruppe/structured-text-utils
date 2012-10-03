/*
 * This define is for Windows only, it is a work-around for bug 661663.
 */
#ifdef _MSC_VER
# define XP_WIN
#endif

/* Include the JSAPI header file to get access to SpiderMonkey. */
#include "jsapi.h"



/* The class of the global object. */
static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

/* The error reporter callback. */
void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    fprintf(stderr, "%s:%u:%s\n",
            report->filename ? report->filename : "<no filename=\"filename\">",
            (unsigned int) report->lineno,
            message);
}

class JSInterpreter;
class Function;

class Function {
    private:
    JSInterpreter& interpreter;
    jsval jsFunction;

    public:
    Function(JSInterpreter&, jsval);
    operator jsval();
    std::string invoke(std::string input, Function transformation);
};

class JSInterpreter {
    public:

    JSRuntime *rt;
    JSContext *cx;
    JSObject  *global;


    JSInterpreter(){
        /* Create a JS runtime. You always need at least one runtime per process. */
        rt = JS_NewRuntime(8 * 1024 * 1024);
        if (rt == NULL)
            throw "can't create runtime";

        /*
         * Create a context. You always need a context per thread.
         * Note that this program is not multi-threaded.
         */
        cx = JS_NewContext(rt, 8192);
        if (cx == NULL)
            throw "can't create runtime";

        JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT);
        JS_SetVersion(cx, JSVERSION_LATEST);
        JS_SetErrorReporter(cx, reportError);

        /*
         * Create the global object in a new compartment.
         * You always need a global object per context.
         */
        global = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
        if (global == NULL)
            throw "can't create runtime";

        /*
         * Populate the global object with the standard JavaScript
         * function and object classes, such as Object, Array, Date.
         */
        if (!JS_InitStandardClasses(cx, global))
            throw "can't create runtime";
    }

    Function evaluateScript(std::string script){
        jsval rval;
        JSBool ok;
        const char *filename = "noname";

        uintN lineno = 0;
        ok = JS_EvaluateScript(cx, global, script.c_str(), script.length(),
                               filename, lineno, &rval);
        if (rval == JSVAL_NULL || rval == JS_FALSE){
            throw "Could not evaluate script";
        }

        return Function(*this, rval);
    }

    char *jsvalToString(jsval val){
        JSString *str;
        str = JS_ValueToString(cx, val);
        return JS_EncodeString(cx, str);
    }

    jsval asJsValue(std::string value){
        JSString *intermediateForm = JS_NewStringCopyN(cx, value.c_str(), value.length());
        return STRING_TO_JSVAL(intermediateForm);
    }

    std::string invoke(Function function, std::string input, Function transformation) {
        jsval r;
        jsval args[] = { asJsValue(input), transformation };
        JS_CallFunctionValue(cx, NULL, function, 2, args, &r);
        return jsvalToString(r);
    }

    ~JSInterpreter(){
        /* Clean things up and shut down SpiderMonkey. */
        JS_DestroyContext(cx);
        JS_DestroyRuntime(rt);
        JS_ShutDown();
    }

};


Function::Function(JSInterpreter &interpreter, jsval jsFunction):
    interpreter(interpreter),
    jsFunction(jsFunction)
{}

Function::operator jsval(){
    return jsFunction;
}

std::string Function::invoke(std::string input, Function transformation) {
    return interpreter.invoke(*this, input, transformation);
}

