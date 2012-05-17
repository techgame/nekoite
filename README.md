## Sample Script Object

        struct ExampleObj : NPScriptObjStd<ExampleObj> {
            static NPScriptClass<ExampleObj> klass;

            ExampleObj(NPP npp, NPClass *aClass)
             : NPScriptObjStd(npp, aClass)
            {
                registerMethod("example", &ExampleObj::example);
            }
            virtual ExampleObj* self() { return this; }
            static const char* s_className() { return "[ExampleObj]"; }

            virtual bool example(NPIdentifier, const NPVariant *args, uint32_t argCount, NPVariant *result) {
                return true; }
        };


### Windows: Export DLL entrypoints with Example.def

Because the npapi headers, the normal `__dllexport` method is unavailable.
Export at least the following three entrypoints:

        LIBRARY "npExample.dll"

        EXPORTS
            NP_GetEntryPoints   @1
            NP_Initialize       @2
            NP_Shutdown         @3
        
### Windows: Resource File

Must have a VERSIONINFO block:


        VS_VERSION_INFO VERSIONINFO
        FILEVERSION 1,0,0,0
        PRODUCTVERSION 1,0,0,0
        FILEFLAGSMASK 0x17L
        #ifdef _DEBUG
        FILEFLAGS 0x1L
        #else
        FILEFLAGS 0x0L
        #endif
        FILEOS 0x40000L
        FILETYPE 0x2L
        FILESUBTYPE 0x0L
        BEGIN
            BLOCK "StringFileInfo"
            BEGIN
                BLOCK "040904E4"
                BEGIN
                    VALUE "CompanyName", "npExample Company"
                    VALUE "FileVersion", "1.0.1"
                    VALUE "InternalName", "npExample.dll"
                    VALUE "LegalCopyright", "Copyright (C) 2012"
                    VALUE "ProductName", "npExample"
                    VALUE "ProductVersion", "1.0.1"
                    VALUE "MIMEType", "application/x-npExampleMimetype"
                END
            END
            BLOCK "VarFileInfo"
            BEGIN
                VALUE "Translation", 0x409, 1200
            END
        END

