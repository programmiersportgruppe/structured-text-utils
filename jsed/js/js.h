#ifndef JS_H_INCLUDED
#define JS_H_INCLUDED
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "jsapi.h"




namespace js {
    class Boolean;
    class String;
    class Array;
    class Function;


    class Value {
        public:

        virtual jsval toJsval(JSContext *cx) const = 0;
        virtual std::string toString() const = 0;

        virtual bool isBoolean() const { return false; }
        virtual bool isString() const { return false; }
        virtual bool isArray() const { return false; }
        virtual bool isFunction() const { return false; }

        virtual Boolean & asBoolean() const { throw * new std::runtime_error("I am not a boolean"); }
        virtual Array & asArray() const { throw * new std::runtime_error("I am not an array"); }
        virtual String & asString() const { throw * new std::runtime_error("I am not a string"); }
        virtual Function & asFunction() const { throw * new std::runtime_error("I am not a function"); }
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

        virtual std::string toString() const {
                return cRep;
        }

        virtual bool isString() const { return true; }
        virtual String & asString() const { return * new String(cRep); }

    };

    class Boolean: public Value {
        private:
        bool cRep;

        public:
        Boolean(bool cRep) : cRep(cRep) { }

        virtual jsval toJsval(JSContext *cx) const {
            return cRep ? JSVAL_TRUE : JSVAL_FALSE;
        }

        virtual std::string toString() const {
                return cRep ? "true":"false";
        }

        virtual bool isBoolean() const { return true; }
        virtual Boolean & asBoolean() const { return * new Boolean(cRep); }

    };

    class Array: public Value {
        private:
        std::vector<ValueRef> cRep;
        public:
        Array(std::vector<ValueRef> cRep) : cRep(cRep)
            { }

        std::vector<ValueRef> elements() {
            return cRep;
        }

        virtual jsval toJsval(JSContext *cx) const {
            return JSVAL_FALSE;
            //todo: implement me
        }

        virtual std::string toString() const {
            std::stringstream ss;
            for(size_t i = 0; i < cRep.size(); ++i)
            {
                if(i != 0)
                ss << ",";
                ValueRef element=cRep[i];
                ss << (*element).toString();
            }
            std::string s = ss.str();
            return s;
        }

        virtual bool isArray() const { return true; }
        virtual Array & asArray() const { return * new Array(cRep); }

    };

    class Function: public Value {
        private:
        JSObject *jsRep;

        public:
        Function(JSObject *jsRep): jsRep(jsRep){}

        virtual jsval toJsval(JSContext *cx) const {
            return OBJECT_TO_JSVAL(jsRep);
        }
        virtual std::string toString() const {
                return "<function>";
        }

        virtual bool isFunction() const {return false; }

        virtual Function & asFunction() const { return * new Function(jsRep); }

    };

}

/* Interface for a function implemented in C and to passed into the interpreter */
class CFunc {
    public:
    virtual js::ValueRef operator() (std::vector<js::ValueRef> args) const = 0;
};

class JSInterpreter {
    private:

    JSRuntime *rt;
    JSContext *cx;
    JSObject  *global;
    char* jsvalToString(const jsval val) const;


    public:

    JSInterpreter();
    JSInterpreter(const JSInterpreter&);
    js::Function evaluateScript(std::string script);

    void registerNativeFunction(std::string name, CFunc *callback);

    std::string invoke(const js::Function function, std::vector<js::ValueRef> args);

    ~JSInterpreter();
};


#endif

