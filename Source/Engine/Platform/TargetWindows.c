#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include "Engine.h"

FGT GEngine = {0};

extern Bool ApiWin32Init(IWindowApi* WindowApi);
extern Void ApiGdiSwapBuffer();

static FILE* SLogFile = NULL;
static String SLogFilePath = "LogFile.txt";
static LARGE_INTEGER STimeFrequency = {0};
static Double SFrameStartTime = 0;

static struct {
  HANDLE hConsole;
  Int32 defaultAttribute;
} SConsole;

static Void InitWin32Console() {
  GEngine.windowApi.bShouldClose = true;
  SConsole.hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO consoleScreenInfo;
  GetConsoleScreenBufferInfo(SConsole.hConsole, &consoleScreenInfo);
  SConsole.defaultAttribute = consoleScreenInfo.wAttributes;
}

Void EngineInit(Int32 Width, Int32 Height, String Title) {
  GEngine.timerApi.engineStartTime = EngineGetTime();
  InitWin32Console();
  QueryPerformanceFrequency(&STimeFrequency);
  SLogFile = fopen(SLogFilePath, "w");
  if(ApiWin32Init(&GEngine.windowApi)) {
    CALL_API(GEngine.windowApi.OnWindowCreate, NULL, Width, Height, Title)
  }
}

Bool EngineShouldClose() {
  return GEngine.windowApi.bShouldClose;
}

Void EngineShutdown() {
  CALL_API(GEngine.windowApi.OnWindowDestroy, NULL);
}

Void EngineBeginFrame() {
  SFrameStartTime = EngineGetTime();
  CALL_API(GEngine.windowApi.OnWindowUpdate, NULL);
}

Void EngineEndFrame() {
  ApiGdiSwapBuffer();
  Double end = EngineGetTime();
  Double delta = end - SFrameStartTime;
  Double target = GEngine.timerApi.frameTime;
  Double remainingTime = target - delta;

  if(target > 0) {
    if(remainingTime > (delta * 0.9)) {
      EngineWait(remainingTime);
    }
    while((delta = EngineGetTime() - SFrameStartTime) < target) {}
  }

  GEngine.timerApi.deltaTime = delta;
  GEngine.timerApi.frameRate = ceil(1.f / delta);
}

Void EngineSetFullscreen(Bool bIsFullscreen) {
  CALL_API(GEngine.windowApi.OnWindowFullscreen, NULL, bIsFullscreen);
}

Double EngineGetTime() {
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return (Double)(counter.QuadPart) / STimeFrequency.QuadPart;
}

Void EngineSetTargetFPS(UInt32 Target) {
  GEngine.timerApi.frameTime = 1.f / Target;
}

Void EngineWait(Double Milliseconds) {
  Sleep(Milliseconds);
}

// Module
Void* EngineLoadModule(String Name) {
  return LoadLibraryA(Name);
}

Void EngineFreeModule(Void* Module) {
  if(Module != NULL) {
    FreeLibrary((HMODULE)Module);
  }
}

Void* EngineGetFunc(Void* Module, String Name) {
  return GetProcAddress((HMODULE)Module, Name);
}

Void EngineLoadApi(Void* Module, Void* Api, String* Names, Bool bDebugMode) {
  String* names = Names;
  Void* function = NULL;
  Int32 index = 0;

  while(**names) {
    function = EngineGetFunc(Module, *names);
    if(function != NULL) {
      Void* addr = (Char*)Api + index * sizeof(Void*);
      memcpy(addr, &function, sizeof(Void*));
      if(bDebugMode) {
        GT_LOG(LOG_INFO, "Function Loaded:%s", *names);
      }
    } else if(bDebugMode) {
      GT_LOG(LOG_INFO, "Function Not Loaded:%s", *names);
    }
    names++;
    index++;
  }
}

Void EnginePrintLog(ELogLevel Level, String Context, String Format, ...) {
  static Char logBuffer[BUFFER_LOG_SIZE] = {""};
  static String logTag = NULL;
  Bool bIsFast = (Level >> 8);
  UInt16 logLevel = Level & 0xFF;

  enum {
    LC_WHITE = 15,
    LC_GREEN = 10,
    LC_YELLOW = 14,
    LC_RED = 12,
    LC_DARK_RED = 4,
  } color;

  switch(logLevel) {
    case LOG_INFO: {
      logTag = (String) "[LOG INFO] =>";
      color = LC_WHITE;
    } break;
    case LOG_SUCCESS: {
      logTag = (String) "[LOG SUCCESS] =>";
      color = LC_GREEN;
    } break;
    case LOG_WARNING: {
      logTag = (String) "[LOG WARNING] =>";
      color = LC_YELLOW;
    } break;
    case LOG_ERROR: {
      logTag = (String) "[LOG ERROR] =>";
      color = LC_RED;
    } break;
    case LOG_FATAL: {
      logTag = (String) "[LOG FATAL] =>";
      color = LC_DARK_RED;
    } break;
  }

  SetConsoleTextAttribute(SConsole.hConsole, color);
  va_list args;
  va_start(args, Format);
  snprintf(logBuffer, sizeof(logBuffer), "%s %s %s\n", logTag, Format, (Level == LOG_INFO) ? "" : Context);
  vprintf(logBuffer, args);
  va_end(args);
  SetConsoleTextAttribute(SConsole.hConsole, SConsole.defaultAttribute);

  if(SLogFile != NULL && bIsFast != 1) {
    va_start(args, Format);
    vfprintf(SLogFile, logBuffer, args);
    va_end(args);
  }
}

#endif  // PLATFORM_WINDOWS
