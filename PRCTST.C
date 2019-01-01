#define PROGRAM       "PROCTEST"         // Program Name

#pragma title (PROGRAM " - Test DosQProcStatus")
#pragma linesize(132)
#pragma pagesize(55)

/***************************************************************************
**            PROCTEST - Test DosQProcStatus API under OS/2 2.0.          **
***************************************************************************/
#define  INCL_DOS                      // Include Base OS/2 Calls
#define  INCL_DOSERRORS                // Include Base OS/2 Errors
#include <os2.h>                       // Standard OS/2 Definitions
#include <stdio.h>                     // C/2 Standard I/O Defs
#include <stdlib.h>                    // C/2 Standard Library
#include <string.h>                    // C/2 Standard String Defs

#define  BUFFERSIZE  (65535)           // Buffer size

#pragma subtitle ("Define Headers and stuff")
#pragma page()
/**************************************************************************
**           The story so far on the DosQProcStatus dataarea             **
**************************************************************************/

/* #pragma linkage( DosQProcStatus, far16 pascal ) */
USHORT APIENTRY DosQProcStatus( PVOID pvBuf, USHORT cbBuf );

#define PROCESS_END_INDICATOR   3      // Indicates end of process structs

#pragma pack(1)

typedef struct _SUMMARY                // Size = 12
{
    ULONG   ulThreadCount;             // Number of threads in system
    ULONG   ulProcessCount;            // Number of processes in system
    ULONG   ulModuleCount;             // Number of modules in system
} SUMMARY, *PSUMMARY;

typedef struct _THREADINFO             // Size = 28
{
    UCHAR   uchDontKnow1;              //
    UCHAR   uchDontKnow2;              //
    USHORT  usDontKnow3;               //
    USHORT  tidWithinProcess;          // TID within process (TID is 4 bytes!!)
    USHORT  tidWithinSystem;           // TID within system
    ULONG   ulBlockId;                 // Block ID (?)
    USHORT  usPriority;                // Priority
    USHORT  usDontKnow4;               //
    USHORT  usDontKnow5;               //
    USHORT  usDontKnow6;               //
    USHORT  usDontKnow7;               //
    USHORT  usDontKnow8;               //
    USHORT  usThreadStatus;            // 2=blocked or ready, 5=running
    USHORT  usDontKnow9;               //
} THREADINFO, *PTHREADINFO;

typedef struct _PROCESSINFO            // Size = 56
{
    ULONG       ulEndIndicator;        // 1 means not end, 3 means last entry
    PTHREADINFO ptiFirst;              // Address of the 1st Thread Control Blk
    USHORT      pid;                   // Process ID (2 bytes - PID is 4 bytes)
    USHORT      pidParent;             // Parent's process ID
    USHORT      usDontKnow2;           //
    USHORT      usDontKnow3;           //
    USHORT      usDontKnow4;           //
    USHORT      usDontKnow5;           //
    USHORT      idSession;             // Session ID
    USHORT      usDontKnow6;           //
    USHORT      hModRef;               // Module handle of EXE
    USHORT      usThreadCount;         // Number of threads in this process
    USHORT      usSessionType;         // Session type (SSF_TYPE_xx)
    CHAR        achDontKnow7[ 6 ];     //
    USHORT      usThreadIdCount;       // Number of USHORTs in Thread id table?
    USHORT      usModCount;            // Number of USHORTs in ModHandle table?
    USHORT      usUshortCount;         // Number of USHORTs in Ushort table?
    USHORT      usDontKnow8;           //
    ULONG       ulThreadIdTableAddr;   // Maybe the address of a TID table
    ULONG       ulModHandleTableAddr;  // Address of a ModHandle table (DLLS?)
    ULONG       ulUshortTableAddr;     // Address of a table of USHORTs (?)
} PROCESSINFO, *PPROCESSINFO;

typedef struct _SEMINFO                // Size = 19 + length of name
{
    struct _SEMINFO *pNext;            // Pointer to next block
    USHORT   idOwningThread;           // ID of owning thread?
    USHORT   fsFlags;                  // (MSB-LSB)
    UCHAR    uchReferenceCount;        // Number of references
    UCHAR    uchRequestCount;          // Number of requests
    CHAR     achDontKnow1[ 6 ];        // Unknown but was 7
    USHORT   usIndex;                  // Index (?)
    CHAR     szSemName[ 1 ];           // ASCIIZ semaphore name
} SEMINFO, *PSEMINFO;

typedef struct _SHRMEMINFO             // Size = 11 + length of name
{
    struct _SHRMEMINFO *pNext;         // Pointer to next block
    USHORT      usMemHandle;           // Shared memory handle (?)
    SEL         selMem;                // Selector
    USHORT      usReferenceCount;      // Number of references
    CHAR        szMemName[ 1 ];        // ASCIIZ shared memory name
} SHRMEMINFO, *PSHRMEMINFO;

typedef struct _MODINFO                // Size = 26 + length of name + 1
{
    struct _MODINFO *pNext;            // Pointer to next block
    USHORT   hMod;                     // Module handle
    USHORT   usModType;                // Module type (0=16bit,1=32bit)
    ULONG    ulModRefCount;            // Count of module references
    ULONG    ulSegmentCount;           // Number of segments in module
    ULONG    ulDontKnow1;              //
    PSZ      szModName;                // Addr of fully qualified module name                                   
    USHORT   usModRef[ 1 ];            // Handles of module references
} MODINFO, *PMODINFO;

typedef struct _BUFFHEADER             // Size = 32
{
    PSUMMARY        psumm;             // Pointer to SUMMARY section
    PPROCESSINFO    ppi;               // Pointer to PROCESS section
    PSEMINFO        psi;               // Pointer to SEMAPHORE section
    PVOID           pDontKnow1;        //
    PSHRMEMINFO     psmi;              // Pointer to SHARED MEMORY section
    PMODINFO        pmi;               // Pointer to MODULE section
    PVOID           pDontKnow2;        //
    PVOID           pDontKnow3;        //
} BUFFHEADER, *PBUFFHEADER;

#pragma pack()

#pragma subtitle ("Define Static & Global Data Areas")
#pragma page()
/**********************************************************************
**                     Define Static Data Areas                      **
**********************************************************************/
static CHAR const szEyePopper[] = PROGRAM;

#pragma subtitle ("Display Modules Section")
#pragma page()
/**********************************************************************
**                       Display Modules Section                     **
**********************************************************************/
VOID DisplayModules (PMODINFO pMod)
  {
  PMODINFO pTmp;                       // Temp ptr

    printf("\n\tModules Section...: %p\n", pMod);
    pTmp = pMod;                       // Get ptr
    while (pTmp->pNext)                // Display Modules
      {
        SELECTOROF(pTmp->szModName) = SELECTOROF(pTmp);  // Fix selector
        SELECTOROF(pTmp->pNext)     = SELECTOROF(pTmp);  // Fix selector
        printf("\t%p ModName: %s\n", pTmp, pTmp->szModName);
        pTmp = pTmp->pNext;                    // Get next ctl block
      }
  }

#pragma subtitle ("Display Shared Memory Section")
#pragma page()
/**********************************************************************
**                    Display Shared Memory Section                  **
**********************************************************************/
VOID DisplayShareMem (PSHRMEMINFO pMem)
  {
  PSHRMEMINFO pTmp;                    // Temp ptr

    printf("\n\tShared Memory Section...: %p\n", pMem);
    pTmp = pMem;                       // Get ptr
    while (pTmp->pNext)                // Display Shared Memory
      {
        SELECTOROF(pTmp->pNext) = SELECTOROF(pMem);    // Fix selector
        printf("\t%p MemName: %s\n", pTmp, pTmp->szMemName);
        pTmp = pTmp->pNext;            // Get next ctl block
      }
  }

#pragma subtitle ("Display Semaphore Section")
#pragma page()
/**********************************************************************
**                      Display Semaphore Section                    **
/*********************************************************************/
VOID DisplaySemaphore (PSEMINFO pSem)
  {
  PSEMINFO pTmp;                       // Temp semaphore

    OFFSETOF(pSem) += 16;              // This must be a bug!
    printf("\n\tSemaphore Section...: %p\n", pSem);
    pTmp = pSem;                       // Need temp ptr due to strangeness
    while (pTmp->pNext)                // Display Semaphores
      {
        SELECTOROF(pTmp->pNext) = SELECTOROF(pSem);    // Fix selector
        printf("\t%p SemName: %s\n", pTmp, pTmp->szSemName);
        pTmp = pTmp->pNext;            // Get next ctl block
      }
  }

#pragma subtitle ("Display Process Section")
#pragma page()
/**********************************************************************
**                       Display Process Section                     **
/*********************************************************************/
VOID DisplayProcess (PPROCESSINFO pProc)
  {
  PTHREADINFO pThread;                 // Thread pointer
  USHORT   i;                          // Index

    printf("\n\tProcess Section...: %p\n", pProc);
    while(pProc->ulEndIndicator != PROCESS_END_INDICATOR)
      {
        printf("\n\t%p Pid=%04d Ppid=%04d Sess=%02d hMod=%04X Tids=%02d Type=%04d\n",
            pProc, pProc->pid, pProc->pidParent, pProc->idSession,
            pProc->hModRef, pProc->usThreadCount, pProc->usSessionType);

        SELECTOROF(pProc->ptiFirst) = SELECTOROF(pProc);   // Fix selector
        pThread = pProc->ptiFirst;     // Get first Thread address

        for (i=0; i < pProc->usThreadCount; i++)   // Loop thru threads
          {
            printf("\t\t%p Tid=%04d SysTid=%04d\n", pThread,
                pThread->tidWithinProcess, pThread->tidWithinSystem);
            pThread++;                 // Bump to next Thread
          }
        pProc = (PPROCESSINFO) pThread;    // Next Process follows Thread
      }
  }

#pragma subtitle ("Display Summary Section")
#pragma page()
/**********************************************************************
**                       Display Summary Section                     **
/*********************************************************************/
VOID DisplaySummary (PSUMMARY pSum)
  {
    printf("\n\tSummary Section:\n");
    printf("\tNumber of threads in system.....: %ld\n", pSum->ulThreadCount);
    printf("\tNumber of processes in system...: %ld\n", pSum->ulProcessCount);
    printf("\tNumber of modules in system.....: %ld\n", pSum->ulModuleCount);
  }

#pragma subtitle ("Display Buffer Header")
#pragma page()
/**********************************************************************
**                        Display Buffer Header                      **
/*********************************************************************/
VOID DisplayBuffHeader (PBUFFHEADER pBufHdr)
  {
    /* Fix screwed-up selector values in buffer header */
    SELECTOROF(pBufHdr->psumm) = SELECTOROF(pBufHdr);  // Fix selector
    SELECTOROF(pBufHdr->ppi)   = SELECTOROF(pBufHdr);  // Fix selector
    SELECTOROF(pBufHdr->psi)   = SELECTOROF(pBufHdr);  // Fix selector
    SELECTOROF(pBufHdr->psmi)  = SELECTOROF(pBufHdr);  // Fix selector
    SELECTOROF(pBufHdr->pmi)   = SELECTOROF(pBufHdr);  // Fix selector

    printf("\n\tBuffer header...........: %p\n", pBufHdr);
    printf("\tSummary Pointer.........: %p\n", pBufHdr->psumm);
    printf("\tProcess Pointer.........: %p\n", pBufHdr->ppi);
    printf("\tSemaphore Pointer.......: %p\n", pBufHdr->psi);
    printf("\tShared Memory Pointer...: %p\n", pBufHdr->psmi);
    printf("\tModule Pointer..........: %p\n", pBufHdr->pmi);
  }

#pragma subtitle ("Main Program")
#pragma page()
/**********************************************************************
**                           Main Entry Point                        **
/*********************************************************************/
USHORT main (USHORT argc, PCHAR argv[])
  {
  USHORT      rc = NO_ERROR;           // Return Code
  USHORT      cbBuffer = BUFFERSIZE;   // Buffer length
  PBUFFHEADER pBuf;                    // Buffer Header pointer
  FILE        *fpTest;                 // File pointer

    OFFSETOF(pBuf) = 0;                // Set ptr offset to zero
    if ((rc = DosAllocSeg(cbBuffer, &SELECTOROF(pBuf), 0))) // Failed
      {
        printf("\a%s: Buffer allocation failed! RC=%d\n", PROGRAM, rc);
        return (rc);                   // Error exit
      }

    memset(pBuf, 0, cbBuffer);         // Zero memory for debugging
    rc = DosQProcStatus(pBuf, cbBuffer);   // Get process data

    if (rc == NO_ERROR)                // Continue
      {
        printf("%s: Size of buffer = %u\n", PROGRAM, cbBuffer);

        if ((fpTest = fopen("PROCTEST.DAT", "wb"))) // File open
          {
            if (fwrite(pBuf, cbBuffer, 1, fpTest) == 1) // Write successful
              {
                DisplayBuffHeader(pBuf);       // Display header
                DisplaySummary(pBuf->psumm);   // Display Summary section
                DisplayProcess(pBuf->ppi);     // Display Process section
                DisplaySemaphore(pBuf->psi);   // Display Semaphore section
                DisplayShareMem(pBuf->psmi);   // Display SharedMem section
                DisplayModules(pBuf->pmi);     // Display Modules section
              }
            else                       // Write failed
              {
                rc = _doserrno;            // Get OS/2 Error
                printf("\a%s: Test file write failed, RC=%d\n", PROGRAM, rc);
              }
            fclose(fpTest);            // Close output file
          }
        else                           // fopen() failed
          {
            rc = _doserrno;            // Get OS/2 error
            printf("\a%s: Test file open failed, RC=%d\n", PROGRAM, rc);
          }
      }
    else                               // DosQProcStatus failed
      {
        printf("\a%s: DosQProcStatus failed, RC=%d\n", PROGRAM, rc);
      }

    return rc;                         // Return To OS/2
  }
