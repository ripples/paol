#include "vgaprocess.h"

VGAProcess::VGAProcess(int camNumIn, int compNum, bool camFlipped, string lecPath)
{
    // Initialize worker
    worker = new VGAWorker(camNumIn, compNum, camFlipped, lecPath);
    // Connect signals to worker and move worker to inner thread
    setupThreadWorkerTimer();
}
