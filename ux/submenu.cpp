#include <string.h>
#include <submenu.h>
#include <alloc.h>
#include <util.h>

Submenu::Submenu(VGAExtended *vga, const char *title)
{
    this->vga = vga;
    strlcpy(this->title, title, 64);
}

void Submenu::attachQueues(QueueHandle_t queueTx, QueueHandle_t queueRx)
{
    this->queueTx = queueTx;
    this->queueRx = queueRx;
}

void Submenu::drawTitle()
{
    vga->print(this->title);
}