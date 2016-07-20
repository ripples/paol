#include "whiteboardprocess.h"

WhiteboardProcess::WhiteboardProcess(string deviceUSB, int camNumIn, int wbNum, bool camFlipped, string lecPath,bool runningGUI)
{
    // Initialize worker
    worker = new WhiteboardWorker(deviceUSB, camNumIn, wbNum, camFlipped, lecPath,runningGUI);
    // Connect signals to worker and move worker to inner thread
    setupThreadWorkerTimer();
}
