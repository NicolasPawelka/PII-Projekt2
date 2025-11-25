#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "logical-dpc.h"
#include "logical-intercore.h"

#include "mt3620-baremetal.h"
#include "mt3620-intercore.h"
#include "mt3620-timer.h"

extern uint32_t StackTop;

// ---------------------------------------------------------------------------
// 1. Vorwärtsdeklarationen
// ---------------------------------------------------------------------------

static _Noreturn void RTCoreMain(void);
static void HandleSendTimerDeferred(void);
static void HandleReceivedMessageDeferred(void);
static _Noreturn void DefaultHandler(void);
static void HandleSendTimerIrq(void);

// ---------------------------------------------------------------------------
// 2. Intercore + Timer Config
// ---------------------------------------------------------------------------

static IntercoreComm icc;
static const uint32_t sendIntervalMs = 1000;

// High-Level-App Component ID
static const ComponentId hlAppId = {
    .data1 = 0x25025d2c,
    .data2 = 0x66da,
    .data3 = 0x4448,
    .data4 = {0xba, 0xe1, 0xac, 0x26, 0xfc, 0xdd, 0x36, 0x27}
};

// ---------------------------------------------------------------------------
// 3. Exception Vector Table (muss bleiben)
// ---------------------------------------------------------------------------

#define INTERRUPT_COUNT 100
#define EXCEPTION_COUNT (16 + INTERRUPT_COUNT)
#define INT_TO_EXC(i_) (16 + (i_))

const uintptr_t ExceptionVectorTable[EXCEPTION_COUNT]
__attribute__((section(".vector_table")))
__attribute__((used)) = {

    [0] = (uintptr_t)&StackTop,
    [1] = (uintptr_t)RTCoreMain,

    [2 ... EXCEPTION_COUNT - 1] = (uintptr_t)DefaultHandler,

    // Timer IRQ
    [INT_TO_EXC(1)] = (uintptr_t)MT3620_Gpt_HandleIrq1,
    // Mailbox IRQ (Intercore)
    [INT_TO_EXC(11)] = (uintptr_t)MT3620_HandleMailboxIrq11,
};

// ---------------------------------------------------------------------------
// 4. IRQ/Timer Handler
// ---------------------------------------------------------------------------

// Deferred Callback für Timer
static void HandleSendTimerIrq(void)
{
    static CallbackNode cb = {.enqueued = false, .cb = HandleSendTimerDeferred};
    EnqueueDeferredProc(&cb);
}

static void HandleSendTimerDeferred(void)
{
    static char msg[] = "RT->HL";
    IntercoreSend(&icc, &hlAppId, msg, sizeof(msg) - 1);

    MT3620_Gpt_LaunchTimerMs(TimerGpt0, sendIntervalMs, HandleSendTimerIrq);
}

// ---------------------------------------------------------------------------
// 5. Intercore Receive Handler (optional, falls HL-App antwortet)
// ---------------------------------------------------------------------------

static void HandleReceivedMessageDeferred(void)
{
    uint8_t rxBuf[32];
    size_t rxSize = sizeof(rxBuf);
    ComponentId sender;

    while (IntercoreRecv(&icc, &sender, rxBuf, &rxSize) == Intercore_OK) {
        // Hier können empfangene Nachrichten verarbeitet werden
    }
}

// ---------------------------------------------------------------------------
// 6. Default Handler
// ---------------------------------------------------------------------------

static _Noreturn void DefaultHandler(void)
{
    for (;;) {
        // do nothing
    }
}

// ---------------------------------------------------------------------------
// 7. Main
// ---------------------------------------------------------------------------

static _Noreturn void RTCoreMain(void)
{
    // Vector Table setzen
    WriteReg32(SCB_BASE, 0x08, (uint32_t)ExceptionVectorTable);

    MT3620_Gpt_Init();
    SetupIntercoreComm(&icc, HandleReceivedMessageDeferred);

    // ersten Timer starten
    MT3620_Gpt_LaunchTimerMs(TimerGpt0, sendIntervalMs, HandleSendTimerIrq);

    for (;;) {
        InvokeDeferredProcs();
        __asm__("wfi");
    }
}
