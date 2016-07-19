#include "whiteboardprocess.h"

WhiteboardProcess::WhiteboardProcess(string deviceUSB, int camNumIn, int wbNum, bool camFlipped, string lecPath)
{
    // Initialize worker
    worker = new WhiteboardWorker(deviceUSB, camNumIn, wbNum, camFlipped, lecPath);
    // Connect signals to worker and move worker to inner thread
    setupThreadWorkerTimer();
}
