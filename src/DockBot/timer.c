/************************************
**
**  DockBot - A Dock For AmigaOS 3
**
**  � 2019 Andrew Kennan
**
************************************/

#include "dock.h"

#include <exec/io.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/intuition_protos.h>

#include "debug.h"

BOOL init_timer_notification(struct DockWindow *dock)
{
    DEBUG(printf("init_timer_notification\n"));

    if( dock->timerPort = CreateMsgPort() ) {

        if( dock->timerReq = (struct timerequest *)CreateExtIO(dock->timerPort, sizeof(struct timerequest) ) ) {
            
            if( OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)dock->timerReq, 0L) == 0 ) {

                return TRUE;
            }
        }
    }

    DEBUG(printf(" Failed\n"));
    return FALSE;
}

VOID free_timer_notification(struct DockWindow *dock)
{
    DEBUG(printf("free_timer_notification\n"));

    if( dock->timerPort ) {

        if( dock->timerReq ) {

            CloseDevice((struct IORequest *)dock->timerReq);    
            DeleteExtIO((struct IORequest *)dock->timerReq);    
        }    

        delete_port(dock->timerPort);
    }
}

VOID set_timer(struct DockWindow *dock, ULONG milliseconds) 
{
    dock->timerReq->tr_node.io_Command = TR_ADDREQUEST;
    dock->timerReq->tr_time.tv_secs = milliseconds / 1000;
    dock->timerReq->tr_time.tv_micro = (milliseconds % 1000) * 1000;

    SendIO((struct IORequest *)dock->timerReq);
}


VOID handle_timer_message(struct DockWindow *dock)
{
    struct DgNode *curr;
    struct Message *msg;

    while( msg = GetMsg(dock->timerPort) ) {
    }

    if( dock->runState == RS_RUNNING
        || dock->runState == RS_CHANGING 
        || dock->runState == RS_EDITING ) {

        FOR_EACH_GADGET(&dock->cfg.gadgets, curr) {

            dock_gadget_tick(curr->dg);
        }
    }

    switch( dock->runState ) {

        case RS_QUITTING:
            dock->runState = RS_STOPPED;
            break;

        case RS_CHANGING:
            handle_change_config(dock);

            set_timer(dock, TIMER_INTERVAL);
            break;            

        case RS_EDITING:
        case RS_RUNNING:
            update_hover_gadget(dock);

            set_timer(dock, TIMER_INTERVAL);
            break;

        case RS_HIDDEN:
            set_timer(dock, TIMER_INTERVAL);
            break;
        
        default:
            break;
    }
}
