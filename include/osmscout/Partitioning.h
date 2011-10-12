#ifndef OSMSCOUT_PARTITIONING_H
#define OSMSCOUT_PARTITIONING_H

#include <QString>

namespace osmscout {
    class Partitioning
    {
    public:
        Partitioning();
        bool PartitionQuality(QString map, QString style, double& result);
    };
}

#endif // OSMSCOUT_PARTITIONING_H
