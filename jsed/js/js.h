#ifndef JS_H_INCLUDED
#define JS_H_INCLUDED
#include <string>
#include <vector>
#include "jsapi.h"


class ContextWrapper;


namespace js {

    class Value {
        public:

        virtual jsval toJsval(JSContext *cx) const = 0;
    };


    class Function;

    class ValueRef {
        private:
        const Value *delegate;
        public:
        ValueRef(const ValueRef &that);
        ValueRef(std::string cRep);
        ValueRef(bool cRep);
        ValueRef(const Value &value);

        ValueRef & operator=(const ValueRef &rhs);

        inline const Value& operator * () {
            return *delegate;
        }
    };


    class String: public Value {
        private:
        std::string cRep;

        public:
        String(std::string cRep) : cRep(cRep) {}

        virtual jsval toJsval(JSContext *cx) const {
            JSString *jsRep;
            jsRep = JS_NewStringCopyN(cx, cRep.c_str(), cRep.length());
            return STRING_TO_JSVAL(jsRep);
        }
    };

    class Boolean: public Value {
        private:
        bool cRep;

        public:
        Boolean(bool cRep) : cRep(cRep) { }

        virtual jsval toJsval(JSContext *cx) const {
            return cRep ? JSVAL_TRUE : JSVAL_FALSE;
        }
    };

    class Function: public Value {
        private:
        JSObject *jsRep;

        public:
        Function(JSObject *jsRep): jsRep(jsRep){}

        virtual jsval toJsval(JSContext *cx) const {
            return OBJECT_TO_JSVAL(jsRep);
        }

    };

}


class JSInterpreter {
    private:

    JSRuntime *rt;
    JSContext *cx;
    JSObject  *global;
    ContextWrapper *cw;


    public:

    JSInterpreter();
    JSInterpreter(const JSInterpreter&);
    js::Function evaluateScript(std::string script);

    std::string invoke(const js::Function function, std::vector<js::ValueRef> args);

    ~JSInterpreter();
};


#endif

