#include "vgaprocess.h"

VGAProcess::VGAProcess(int camNumIn, int compNum, bool camFlipped, string lecPath, bool runningGUI)
{
    // Initialize worker
    worker = new VGAWorker(camNumIn, compNum, camFlipped, lecPath,runningGUI);
    // Connect signals to worker and move worker to inner thread
    setupThreadWorkerTimer();
}
