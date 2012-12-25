/*
 * This define is for Windows only, it is a work-around for bug 661663.
 */
#ifdef _MSC_VER
# define XP_WIN
#endif

/* Include the JSAPI header file to get access to SpiderMonkey. */
#include "jsapi.h"
#include "filter/filter.h"


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

class ContextWrapper {
    JSContext *cx;

    public:
    ContextWrapper(JSContext *cx):
        cx(cx)
    {}

};

class JSInterpreter;

class Function {
    private:
    JSInterpreter& interpreter;
    jsval jsFunction;

    public:
    Function(JSInterpreter&, jsval);
    operator jsval() const;
    std::string invoke(std::string input, Function transformation, bool raw, bool pretty) const;
};

JSBool _native_filter(JSContext *cx, uintN argc, jsval *vp)
{
    if (argc != 3) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        JS_ReportError(cx, "_native_filter takes three arguments");
        return JS_FALSE;
    }
    std::string input = JS_EncodeString(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[0]));
    std::string command = JS_EncodeString(cx, JS_ValueToString(cx, JS_ARGV(cx, vp)[1]));
    printf("%s | %s\n", input.c_str(), command.c_str());

    JSObject* jsArgs;
    jsval jsvalArgs=JS_ARGV(cx, vp[2]);
    if(!JSVAL_IS_PRIMITIVE(jsvalArgs)){
        printf("jsvalags is not primitive");
    }
    jsArgs = JSVAL_TO_OBJECT(jsvalArgs);
    //JS_ValueToObject(cx, jsvalArgs, &jsArgs);
    //
    jsval r;
    if (!JS_CallFunctionName(cx, JS_GetGlobalObject(cx), "toString", 0, NULL, &r))
        printf("Failed to call to string\n");
    if (JS_IsArrayObject(cx, jsArgs) == JS_FALSE) {
        JS_ReportError(cx, "Third argument to _native_filter must be an array");
        return JS_FALSE;
    }
    std::vector<std::string> args;
    printf("Lo and behold\n");
    jsuint argsArrayLength;
    JS_GetArrayLength(cx, jsArgs, &argsArrayLength);
    printf("%i\n", argsArrayLength);


    for (int i = 0; i < argsArrayLength; i++){
        jsval ret;
        JS_GetElement(cx, jsArgs, i, &ret);

        args.push_back(JS_EncodeString(cx, JS_ValueToString(cx, ret)));
    }
    printf("echo '%s' | %s\n", input.c_str(), command.c_str());


    std::string output = filter(input, command, args);
    //try catch to handle error
    JSString *intermediateForm = JS_NewStringCopyN(cx, output.c_str(), output.length());
    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(intermediateForm));

    return JS_TRUE;
}

class JSInterpreter {
    JSRuntime *rt;
    JSContext *cx;
    JSObject  *global;
    ContextWrapper *contextWrapper;

    public:

    JSInterpreter() {
        /* Create a JS runtime. You always need at least one runtime per process. */
        rt = JS_NewRuntime(8 * 1024 * 1024);
        if (rt == NULL)
            throw * new std::runtime_error("Can't create JS runtime.");

        /*
         * Create a context. You always need a context per thread.
         * Note that this program is not multi-threaded.
         */
        cx = JS_NewContext(rt, 8192);
        if (cx == NULL)
            throw * new std::runtime_error("Can't create js context.");

        JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT);
        JS_SetVersion(cx, JSVERSION_LATEST);
        JS_SetErrorReporter(cx, reportError);

        /*
         * Create the global object in a new compartment.
         * You always need a global object per context.
         */
        global = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
        if (global == NULL)
            throw * new std::runtime_error("Can't create global object.");

        /*
         * Populate the global object with the standard JavaScript
         * function and object classes, such as Object, Array, Date.
         */
        if (!JS_InitStandardClasses(cx, global))
            throw * new std::runtime_error("Can't initialise standard classes.");

        if (!JS_DefineFunction(cx, global, "_native_filter", &_native_filter, 3, 0))
            throw * new std::runtime_error("Can't define function.");

        contextWrapper = new ContextWrapper(cx);

    }

    Function evaluateScript(std::string script){
        jsval rval;
        JSBool ok;
        const char *filename = "noname";

        uintN lineno = 0;
        ok = JS_EvaluateScript(cx, global, script.c_str(), script.length(),
                               filename, lineno, &rval);
        if (rval == JSVAL_NULL || rval == JS_FALSE || ok == JS_FALSE){
            throw * new std::runtime_error("Could not evaluate script");
        }

        return Function(*this, rval);
    }


    jsval fromBool(bool b){
        return b ? JSVAL_TRUE : JSVAL_FALSE;
    }

    std::string invoke(const Function function, std::string input, Function transformation, bool raw, bool pretty) {
        jsval r;
        jsval args[] = { asJsValue(input), transformation, fromBool(raw), fromBool(pretty)};
        JS_CallFunctionValue(cx, NULL, function, 4, args, &r);
        return jsvalToString(r);
    }

    ~JSInterpreter() {
        /* Clean things up and shut down SpiderMonkey. */
        JS_DestroyContext(cx);
        JS_DestroyRuntime(rt);
        JS_ShutDown();
    }

    private:

    JSInterpreter(const JSInterpreter& that);

    char *jsvalToString(const jsval val) const {
        JSString *str;
        str = JS_ValueToString(cx, val);
        return JS_EncodeString(cx, str);
    }

    jsval asJsValue(const std::string value) const {
        JSString *intermediateForm = JS_NewStringCopyN(cx, value.c_str(), value.length());
        return STRING_TO_JSVAL(intermediateForm);
    }

};


Function::Function(JSInterpreter &interpreter, jsval jsFunction):
    interpreter(interpreter),
    jsFunction(jsFunction)
{}

Function::operator jsval() const {
    return jsFunction;
}

std::string Function::invoke(std::string input, Function transformation, bool raw, bool pretty) const {
    return interpreter.invoke(*this, input, transformation, raw, pretty);
}

