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
    ValueRef::ValueRef(std::string cRep): delegate(new String(cRep)){
    };

    // We are leaking memory.
    ValueRef::ValueRef(bool cRep): delegate(new Boolean(cRep)){
    };

    ValueRef::ValueRef(const Value &value): delegate(&value){
    };

};


/* This is the global native function handler registry */

std::map<int, CFunc *> functionHandlers;


js::ValueRef wrap(JSContext *cx, jsval val) {

    if ( JSVAL_IS_BOOLEAN(val)) {
        return JSVAL_TO_BOOLEAN(val) ? true : false;
    }
    if ( JSVAL_IS_NULL(val)){
     fprintf(stderr, "is null\n");
     }
    if ( JSVAL_IS_VOID(val)){
     fprintf(stderr, "is void\n");
     }
    if ( JSVAL_IS_INT (val)){
     fprintf(stderr, "is int\n");
     }
    if ( JSVAL_IS_STRING(val)) {
        JSString *str = JSVAL_TO_STRING(val);
        //todo: memory leak is my middle name
        js::String *jsstr = new js::String((std::string)JS_EncodeString(cx, str));

        return *jsstr;
    }
    if ( !JSVAL_IS_PRIMITIVE(val)) {
        JSObject* obj=JSVAL_TO_OBJECT(val);
        if (JS_IsArrayObject(cx, obj)){
            std::vector<js::ValueRef> els;
            jsuint length;
            JS_GetArrayLength(cx, obj, &length);
            for (int i = 0; i < length; i++){
                jsval ret;
                JS_GetElement(cx, obj, i, &ret);
                els.push_back(wrap(cx, ret));
            }
            // leaking memory
            js::Array *array=new js::Array(els);
            return (*array);
        }
    }
    return (std::string)"<unknown type>";
}

JSBool globalFunctionCallBack(JSContext *cx, uintN argc, jsval *vp) {
    jsval func=  JS_CALLEE(cx, vp);
    JSObject *funcO = JSVAL_TO_OBJECT(func);
    CFunc *callback = functionHandlers[(long)funcO];
    //todo handle failed lookup.
    std::vector<js::ValueRef> args;
    try {
        for (int i = 0; i < argc; i++) {
            args.push_back(wrap(cx, JS_ARGV(cx, vp)[i]));
        }
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

char *JSInterpreter::jsvalToString(const jsval val) const {
        JSString *str;
        str = JS_ValueToString(cx, val);
        return JS_EncodeString(cx, str);
}

std::string JSInterpreter::invoke(js::Function function, std::vector<js::ValueRef> args) {
    jsval r;

    jsval jsArgs[args.size()];
    for (int i = 0; i < args.size(); i++) {
      jsArgs[i] = (*args[i]).toJsval(cx);

    }

    JS_CallFunctionValue(cx, NULL, function.toJsval(cx), args.size(), jsArgs, &r);
    return jsvalToString(r);
}

JSInterpreter::~JSInterpreter() {
    /* Clean things up and shut down SpiderMonkey. */
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
}

