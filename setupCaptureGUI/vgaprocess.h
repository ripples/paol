#ifndef VGAPROCESS_H
#define VGAPROCESS_H

#include "paolProcess.h"
#include "vgaworker.h"

class VGAProcess : public paolProcess
{
public:
    VGAProcess(int camNumIn, int vgaNum, bool camFlipped, string lecturePath);
};

#endif // VGAPROCESS_H
