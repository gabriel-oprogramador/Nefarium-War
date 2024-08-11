#include "Engine.h"

// Game Entrypoint
extern Void GameInit();
extern Void GameStart();
extern Void GameUpdate(Float DeltaTime);
extern Void GameStop();

static Bool SbInitGamePlay = true;
static Bool SbGameIsRunning = false;

// Engine Entrypoint
int GTmain(int argc, const char** argv) {
  EngineInit(1280, 768, STR(GAME_NAME));
  GameInit();

  while (!EngineShouldClose()) {
    EngineBeginFrame();
    if (SbInitGamePlay && !SbGameIsRunning) {
      SbGameIsRunning = true;
      GameStart();
    }
    if (SbInitGamePlay) {
      GameUpdate(0.016f);
    }
    EngineEndFrame();
  }
  EngineShutdown();

  return 0;
}

// Platform Entrypoint
#if defined(PLATFORM_WINDOWS) && defined(RELEASE_MODE)
#include <windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  printf("Entrypoint => WinMain\n");
  return GTmain(0, (const char**)lpCmdLine);
}
#elif defined(PLATFORM_LINUX) || defined(DEBUG_MODE)
int main(int argc, const char** argv) {
  printf("Entrypoint => Main\n");
  return GTmain(argc, argv);
}
#endif
