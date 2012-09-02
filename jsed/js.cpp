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

    jsval evaluateScript(const char *script){
        jsval rval;
        JSBool ok;
        const char *filename = "noname";

        uintN lineno = 0;
        ok = JS_EvaluateScript(cx, global, script, strlen(script),
                               filename, lineno, &rval);
        if ((rval == NULL) || rval == JS_FALSE){
            throw "Could not evaluate script";
        }

        return rval;
    }

    jsval executeScriptFile(const char *script){
        jsval rval;

        JSObject *jsedScript = JS_CompileFile(cx, global , "jsed.js");
        JS_ExecuteScript(cx, global, jsedScript, &rval);

        if (rval == NULL || rval == JS_FALSE){
            throw "Could not evaluate script";
        }
        return rval;
    }

    char *jsvalToString(jsval val){
        JSString *str;
        str = JS_ValueToString(cx, val);
        return JS_EncodeString(cx, str);
    }

    void setProperty(char *name, jsval *val){
        JS_SetProperty(cx, global, name, val);
    }

    void setProperty(char *name, const char *value){
        JSString *inputJs;
        inputJs = JS_NewStringCopyN(cx, value, strlen(value));
        jsval inputJsVal;
        inputJsVal=STRING_TO_JSVAL(inputJs);
        JS_SetProperty(cx, global, name, &inputJsVal);
    }

    ~JSInterpreter(){
        /* Clean things up and shut down SpiderMonkey. */
        JS_DestroyContext(cx);
        JS_DestroyRuntime(rt);
        JS_ShutDown();
    }

};


