#include <algorithm>
#include <cstdio>

#include "imagestorage.h"
#include "standardimagestorage.h"
#include "jispiconset.h"
#include "log.h"
#include "cfg.h"

namespace SIM {

static ImageStorage* g_imageStorage = 0;

EXPORT ImageStorage* getImageStorage()
{
    return g_imageStorage;
}

void EXPORT setImageStorage(ImageStorage* storage)
{
    g_imageStorage = storage;
}

void EXPORT createImageStorage()
{
    Q_ASSERT(!g_imageStorage);
    g_imageStorage = new StandardImageStorage();
}

void EXPORT destroyImageStorage()
{
    Q_ASSERT(g_imageStorage);
    delete g_imageStorage;
    g_imageStorage = 0;
}

} // namespace SIM
