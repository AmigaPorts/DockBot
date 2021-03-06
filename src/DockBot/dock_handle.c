/************************************
**
**  DockBot - A Dock For AmigaOS 3
**
**  � 2019 Andrew Kennan
**
************************************/

#include "dock.h"

#include <intuition/intuition.h>
#include <intuition/classes.h>

#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "dock_handle.h"

#define HANDLE_SIZE 6

struct DockHandleData {
    UWORD counter;
};


VOID dock_handle_draw(Class *c, Object *o, struct RastPort *rp)
{
    struct DockHandleData *dhd;
    struct GadgetEnvironment env;
    struct Screen *screen;
    struct DrawInfo *drawInfo;

    dhd = INST_DATA(c,o);

    DB_GetDockGadgetEnvironment(o, &env);

    if( screen = LockPubScreen(NULL) ) {
    
        if( drawInfo = GetScreenDrawInfo(screen) ) {

            SetAPen(rp, drawInfo->dri_Pens[BACKGROUNDPEN]);
            RectFill(rp, env.gadgetBounds.x, env.gadgetBounds.y, 
                env.gadgetBounds.x + env.gadgetBounds.w - 1,
                env.gadgetBounds.y + env.gadgetBounds.h - 1);
          
            FreeScreenDrawInfo(screen, drawInfo);
        }

        UnlockPubScreen(NULL, screen);
    }     

    if( dhd->counter == 0 ) {
        DB_DrawOutsetFrame(rp, &env.gadgetBounds);
    } else {
        DB_DrawInsetFrame(rp, &env.gadgetBounds);
    }
}

ULONG __saveds dock_handle_dispatch(Class *c, Object *o, Msg msg)
{
    struct DockHandleData *dhd;
    struct DockMessageDraw *dm;
    struct DockMessageGetSize *gsm;

    switch( msg->MethodID ) 
    {

        case DM_CLICK:
            dhd = INST_DATA(c,o);
            dhd->counter = 2;
            DB_RequestDockGadgetDraw(o);
            break;

        case DM_TICK:
            dhd = INST_DATA(c,o);
            if( dhd->counter > 0 ) {
                dhd->counter--;
                if( dhd->counter == 0 ) {
                    DB_RequestDockGadgetDraw(o);
                }
            }
            break;

        case DM_DRAW:
            dm = (struct DockMessageDraw *)msg;
            dock_handle_draw(c, o, dm->rp);
            break;

        case DM_GETSIZE:
            gsm = (struct DockMessageGetSize *)msg;
            switch( gsm->position ) {
                case DP_TOP:
                case DP_BOTTOM:
                    gsm->w = HANDLE_SIZE;
                    gsm->h = DEFAULT_SIZE;
                    break;
                case DP_LEFT:
                case DP_RIGHT:
                    gsm->w = DEFAULT_SIZE;
                    gsm->h = HANDLE_SIZE;
                    break;
            }
            break;

        case DM_BUILTIN:
            return (ULONG)TRUE;

        default:
            return DoSuperMethodA(c, o, msg);
    }

    return NULL;
}

Class *init_dock_handle_class(VOID) 
{
    ULONG HookEntry();
    Class *c;

    DEBUG(printf("init_dock_handle_class\n"));

    if( c = MakeClass(HANDLE_CLASS_NAME, DB_ROOT_CLASS, NULL, sizeof(struct DockHandleData), 0) )
    {
        DEBUG(printf("Class %s registered\n", HANDLE_CLASS_NAME));

        c->cl_Dispatcher.h_Entry = HookEntry;
        c->cl_Dispatcher.h_SubEntry = dock_handle_dispatch;

        AddClass(c);
    }

    return c;
}

BOOL free_dock_handle_class(Class *c)
{
    return FreeClass(c);
}
