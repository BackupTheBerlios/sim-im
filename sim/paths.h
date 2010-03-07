
#ifndef SIM_PATHS_H
#define SIM_PATHS_H

#include "cfg.h"

#include <QString>
#include "simapi.h"

namespace SIM
{
    class EXPORT PathManager
    {
    public:
        static QString configRoot();
    };
}

#endif

// vim: set expandtab:
