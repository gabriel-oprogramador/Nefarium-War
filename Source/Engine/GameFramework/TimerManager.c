#include "GameFramework.h"
#include "GT/Engine.h"

Void FTimerUpdate(Float DeltaTime) {
  // Call timers update
}

Double FTimerGetDeltaTime() {
  return GEngine.timerApi.deltaTime;
}

UInt32 FTimerGetFrameRate() {
  return GEngine.timerApi.frameRate;
}
