#pragma once

#include "sspf.h"
#include "threadsafequeue.h"
#include "shuffle.h"

struct Fetcher;
class PEffect;
class SVEffect;
class DVEffect;
class Context;

namespace Loader
{

void    SubmitJob(Context *context, uint32 x, uint32 y, bool dontWait, bool showBlack);

void    InitFetchers();
void    ShutdownFetchers();
void    SetHWnd(HWND hWnd);

bool    StartThread();
void    StopThread();

void    SetCallbacks(void (* setEffectCallback)(Context &context, PEffect *peffect, VEffect *veffect),
                    void (* textureLoadCallback)(Context &context, Texture &texture));
void    SetQueueSize(uint32 queueMin, uint32 queueMax);

uint32  AddResources(std::string const &resourceStr);
uint32  FinalizeResources(uint32 volatility);

uint32  FindNearestAspect(uint32 aspect);

void    InitEffects(bool previewMode, uint32 volatility);

};
