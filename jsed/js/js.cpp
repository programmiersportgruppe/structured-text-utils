#include "jsapi.h"
#include "js.h"
#include <string>
#include <vector>
#include <map>
#include <stdexcept>


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


namespace js {
    ValueRef::ValueRef(const ValueRef &that): delegate(that.delegate) {};

    ValueRef & ValueRef::operator=(const ValueRef &rhs){delegate=rhs.delegate;};

    // We are leaking memory.
    ValueRef::ValueRef(std::string cRep): delegate(new String(cRep)){ };

    // We are leaking memory.
    ValueRef::ValueRef(bool cRep): delegate(new Boolean(cRep)){ };

    ValueRef::ValueRef(const Value &value): delegate(&value){ };

};

class ContextWrapper {
    JSContext *cx;

    public:
    ContextWrapper(JSContext *cx):
        cx(cx)
    {}

    char *jsvalToString(const jsval val) const {
        JSString *str;
        str = JS_ValueToString(cx, val);
        return JS_EncodeString(cx, str);
    }
};


/* This is the global native function handler registry */

std::map<int, CFunc *> functionHandlers;


JSBool globalFunctionCallBack(JSContext *cx, uintN argc, jsval *vp){
    jsval func=  JS_CALLEE(cx, vp);
    JSObject *funcO = JSVAL_TO_OBJECT(func);
    CFunc *callback = functionHandlers[(long)funcO];
    //todo handle failed lookup.
    std::vector<js::ValueRef> args;
    try {
        js::ValueRef returnValue = (*callback)(args);
        JS_SET_RVAL(cx, vp, (*returnValue).toJsval(cx));
        return JS_TRUE;
    }
    catch(std::runtime_error& ex){
        JS_SET_RVAL(cx, vp, JSVAL_NULL);

        std::string message;
        JS_ReportError(cx, (message + "Native function failed:\n" + ex.what()).c_str() );
        return JS_FALSE;
    }
}


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


    std::string output = "output";//filter(input, command, args);
    //try catch to handle error
    JSString *intermediateForm = JS_NewStringCopyN(cx, output.c_str(), output.length());
    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(intermediateForm));

    return JS_TRUE;
}

void JSInterpreter::registerNativeFunction(std::string name, CFunc *callback) {
    JSFunction *jsfunc = JS_DefineFunction(cx, global, name.c_str(), &globalFunctionCallBack, 2, 0);
    if (!jsfunc)
            throw * new std::runtime_error("Can't define function '"+name+"'.");

    functionHandlers[(long)jsfunc]=callback;
}

JSInterpreter::JSInterpreter() {
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


        cw = new ContextWrapper(cx);

    }

js::Function JSInterpreter::evaluateScript(std::string script){
    jsval rval;
    JSBool ok;
    const char *filename = "noname";

    uintN lineno = 0;
    ok = JS_EvaluateScript(cx, global, script.c_str(), script.length(),
                           filename, lineno, &rval);
    if (rval == JSVAL_NULL || rval == JS_FALSE || ok == JS_FALSE){
        throw * new std::runtime_error("Could not evaluate script");
    }

    JSObject *obj;
    JS_ValueToObject(cx, rval, &obj);
    JSBool func = JS_ObjectIsFunction(cx, obj);
    //todo guard!
    return js::Function(obj);
}

std::string JSInterpreter::invoke(js::Function function, std::vector<js::ValueRef> args) {
    jsval r;

    jsval jsArgs[args.size()];
    for (int i = 0; i < args.size(); i++) {
      jsArgs[i] = (*args[i]).toJsval(cx);

    }

    JS_CallFunctionValue(cx, NULL, function.toJsval(cx), args.size(), jsArgs, &r);
    return cw->jsvalToString(r);
}

JSInterpreter::~JSInterpreter() {
    /* Clean things up and shut down SpiderMonkey. */
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
}

