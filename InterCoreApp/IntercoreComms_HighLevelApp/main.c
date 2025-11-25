#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <applibs/log.h>
#include <applibs/application.h>
#include "eventloop_timer_utilities.h"

// Exitcodes
typedef enum {
    ExitCode_Success = 0,
    ExitCode_TermHandler_SigTerm,
    ExitCode_TimerHandler_Consume,
    ExitCode_SendMsg_Send,
    ExitCode_SocketHandler_Recv,
    ExitCode_Init_EventLoop,
    ExitCode_Init_SendTimer,
    ExitCode_Init_Connection,
    ExitCode_Init_SetSockOpt,
    ExitCode_Init_RegisterIo,
    ExitCode_Main_EventLoopFail
} ExitCode;

static int sockFd = -1;
static EventLoop *eventLoop = NULL;
static EventLoopTimer *sendTimer = NULL;
static EventRegistration *socketEventReg = NULL;
static volatile sig_atomic_t exitCode = ExitCode_Success;

// RT-App Component ID
static const char rtAppComponentId[] = "005180bc-402f-4cb3-a662-72937dbcde47";

// Forward declarations
static void TerminationHandler(int signalNumber);
static void SendTimerEventHandler(EventLoopTimer *timer);
static void SendMessageToRTApp(void);
static void SocketEventHandler(EventLoop *el, int fd, EventLoop_IoEvents events, void *context);
static ExitCode InitHandlers(void);
static void CloseHandlers(void);

// -------------------------
// Signal handler
// -------------------------
static void TerminationHandler(int signalNumber) {
    exitCode = ExitCode_TermHandler_SigTerm;
}

// -------------------------
// Timer Handler
// -------------------------
static void SendTimerEventHandler(EventLoopTimer *timer) {
    if (ConsumeEventLoopTimerEvent(timer) != 0) {
        exitCode = ExitCode_TimerHandler_Consume;
        return;
    }
    SendMessageToRTApp();
}

// -------------------------
// Send Nachricht an RT-App
// -------------------------
static void SendMessageToRTApp(void) {
    static int iter = 0;
    static char txMessage[32];
    snprintf(txMessage, sizeof(txMessage), "hl-app-to-rt-app-%02d", iter);
    iter = (iter + 1) % 100;

    Log_Debug("Sending: %s\n", txMessage);

    int bytesSent = send(sockFd, txMessage, strlen(txMessage), 0);
    if (bytesSent == -1) {
        Log_Debug("ERROR: Unable to send message: %d (%s)\n", errno, strerror(errno));
        exitCode = ExitCode_SendMsg_Send;
    }
}

// -------------------------
// Socket Handler für RT-App Antworten
// -------------------------
static void SocketEventHandler(EventLoop *el, int fd, EventLoop_IoEvents events, void *context) {
    char rxBuf[32];
    int bytesReceived = recv(fd, rxBuf, sizeof(rxBuf), 0);

    if (bytesReceived == -1) {
        Log_Debug("ERROR: Unable to receive message: %d (%s)\n", errno, strerror(errno));
        exitCode = ExitCode_SocketHandler_Recv;
        return;
    }

    Log_Debug("Received %d bytes: ", bytesReceived);
    for (int i = 0; i < bytesReceived; ++i) {
        Log_Debug("%c", isprint(rxBuf[i]) ? rxBuf[i] : '.');
    }
    Log_Debug("\n");
}

// -------------------------
// Initialisierung
// -------------------------
static ExitCode InitHandlers(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    eventLoop = EventLoop_Create();
    if (!eventLoop) return ExitCode_Init_EventLoop;

    static const struct timespec sendPeriod = {.tv_sec = 1, .tv_nsec = 0};
    sendTimer = CreateEventLoopPeriodicTimer(eventLoop, &SendTimerEventHandler, &sendPeriod);
    if (!sendTimer) return ExitCode_Init_SendTimer;

    sockFd = Application_Connect(rtAppComponentId);
    if (sockFd == -1) return ExitCode_Init_Connection;

    static const struct timeval recvTimeout = {.tv_sec = 5, .tv_usec = 0};
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout)) == -1)
        return ExitCode_Init_SetSockOpt;

    socketEventReg = EventLoop_RegisterIo(eventLoop, sockFd, EventLoop_Input, SocketEventHandler, NULL);
    if (!socketEventReg) return ExitCode_Init_RegisterIo;

    return ExitCode_Success;
}

// -------------------------
// Cleanup
// -------------------------
static void CloseFdAndPrintError(int fd, const char *fdName) {
    if (fd >= 0 && close(fd) != 0) {
        Log_Debug("ERROR: Could not close fd %s: %s (%d).\n", fdName, strerror(errno), errno);
    }
}

static void CloseHandlers(void) {
    DisposeEventLoopTimer(sendTimer);
    EventLoop_UnregisterIo(eventLoop, socketEventReg);
    EventLoop_Close(eventLoop);
    CloseFdAndPrintError(sockFd, "Socket");
}

// -------------------------
// Main
// -------------------------
int main(void) {
    Log_Debug("High-level intercore comms application\n");

    exitCode = InitHandlers();
    while (exitCode == ExitCode_Success) {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        if (result == EventLoop_Run_Failed && errno != EINTR) {
            exitCode = ExitCode_Main_EventLoopFail;
        }
    }

    CloseHandlers();
    Log_Debug("Application exiting.\n");
    return exitCode;
}
