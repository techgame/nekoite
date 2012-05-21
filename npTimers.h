/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#pragma once
#include "npObjFramework.h"

namespace Nekoite {
    /*~ Timer re-association logic ~~~~~~~~~~~~~~~~~~~*/

    struct NPTimerCtx;
    struct NPTimerTarget {
        virtual ~NPTimerTarget() {}
        virtual bool timerRetain(NPTimerCtx* ctx) { return true; }
        virtual bool timerRelease(NPTimerCtx* ctx) { return true; }
        virtual void timerTick(NPTimerCtx* ctx, NPP npp, uint32_t timerID) = 0;
    };

    struct NPTimerCtx {
        typedef std::map<uint32_t, NPTimerCtx> map;

        NPTimerTarget* tgt;
        uint32_t timerID;
        bool repeat;

        NPTimerCtx(NPTimerTarget* tgt_p=NULL, uint32_t timerID_p=0, bool repeat_p=false)
         : tgt(tgt_p), timerID(timerID_p), repeat(repeat_p)
        {
            if (tgt) tgt->timerRetain(this);
        }
        NPTimerCtx(const NPTimerCtx& cpy)
         : tgt(cpy.tgt), timerID(cpy.timerID), repeat(cpy.repeat)
        {
            if (tgt) tgt->timerRetain(this);
        }
        ~NPTimerCtx() {
            if (tgt && !tgt->timerRelease(this))
                delete tgt;
            tgt = NULL;
        }
        bool tick(NPP npp, uint32_t timerID) {
            if (tgt) tgt->timerTick(this, npp, timerID);
            return repeat;
        }
    };
    
    struct NPTimerMgr {
        static NPTimerCtx::map ctxmap;

        static uint32_t scheduleTimer(NPHostObj* host, NPTimerTarget* tgt, uint32_t interval, bool repeat=true) {
            if (host && tgt) {
                uint32_t timerID = host->scheduleTimer(interval, repeat, _on_timer);
                ctxmap[timerID] = NPTimerCtx(tgt, timerID, repeat);
                return timerID;
            } else return 0; }
        static bool unscheduleTimer(NPHostObj* host, uint32_t timerID) {
            if (host && (0<ctxmap.count(timerID))) {
                host->unscheduleTimer(timerID);
                return ctxmap.erase(timerID)>0;
            } else return false; }
        static bool unscheduleTimer(NPHostObj* host, NPTimerCtx* ctx) {
            return ctx ? unscheduleTimer(host, ctx->timerID) : false; }

        static void _on_timer(NPP npp, uint32_t timerID) {
            if (ctxmap.count(timerID)==0) return;
            NPTimerCtx& ctx = ctxmap[timerID];
            if (!ctx.tick(npp, timerID))
                ctxmap.erase(timerID);
        }
    };

    template<typename T>
    struct NPTimerTargetEx : NPTimerTarget {
        NPTimerCtx* timerCtx;
        NPTimerTargetEx():timerCtx(NULL) {}
        virtual bool timerRetain(NPTimerCtx* ctx) { timerCtx = ctx; return true; }
        bool unscheduleTimerEx() { NPTimerCtx* ctx=timerCtx; timerCtx = NULL;
            return NPTimerMgr::unscheduleTimer(dynamic_cast<T*>(this)->host, ctx); }
    };
}
