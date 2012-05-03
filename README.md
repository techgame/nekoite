
## Sample Script Object

        struct ExampleObj : NPScriptObjStd<ExampleObj> {
            ExampleObj(NPP npp, NPClass *aClass)
                : NPScriptObjStd(npp, aClass)
            {
                registerMethod("example", &ExampleObj::example);
            }
            virtual ExampleObj* self() { return this; }
            static const char* s_className() { return "[ExampleObj]"; }

            virtual bool example(NPIdentifier, const NPVariant *args, uint32_t argCount, NPVariant *result) {
                return true;
            }
        };
        NPScriptClass<ExampleObj> exampleClass;


## Create Example.def

        LIBRARY "npExample.dll"

        EXPORTS
            NP_GetEntryPoints   @1
            NP_Initialize       @2
            NP_Shutdown         @3
        
