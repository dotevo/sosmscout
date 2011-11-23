QT       += core sql \
            gui

TARGET = build/example-build
TEMPLATE = app

HEADERS += \
    mainwindow.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    ../src/osmscout/Database.cpp \
    ../src/osmscout/StyleConfigLoader.cpp \
    ../src/osmscout/Util.cpp \
    ../src/osmscout/util/Geometry.cpp \
    ../src/osmscout/StyleConfig.cpp \
    ../src/osmscout/TypeConfig.cpp \
    ../src/osmscout/OptimizeLowZoom.cpp \
    ../src/osmscout/NodeIndex.cpp \
    ../src/osmscout/WayIndex.cpp \
    ../src/osmscout/AreaAreaIndex.cpp \
    ../src/osmscout/AreaNodeIndex.cpp \
    ../src/osmscout/AreaWayIndex.cpp \
    ../src/osmscout/CityStreetIndex.cpp \
    ../src/osmscout/NodeUseIndex.cpp \
    ../src/osmscout/WaterIndex.cpp \
    ../src/osmscout/Route.cpp \
    ../src/osmscout/util/Cache.cpp \
    ../src/osmscout/RoutingProfile.cpp \
    ../src/osmscout/TypeConfigLoader.cpp \
    ../src/osmscout/util/StopClock.cpp \
    ../src/osmscout/oss/Parser.cpp \
    ../src/osmscout/oss/Scanner.cpp \
    ../src/osmscout/Point.cpp \
    ../src/osmscout/util/Projection.cpp \
    ../src/osmscout/Types.cpp \
    ../src/osmscout/Tag.cpp \
    ../src/osmscout/util/Reference.cpp \
    ../src/osmscout/Node.cpp \
    ../src/osmscout/Relation.cpp \
    ../src/osmscout/Way.cpp \
    ../src/osmscout/util/FileScanner.cpp \
    ../src/osmscout/util/String.cpp \
    ../src/osmscout/util/Transformation.cpp \
    ../src/osmscout/NumericIndex.cpp \
    ../src/osmscout/AdminRegion.cpp \
    ../src/osmscout/Location.cpp \
    ../src/osmscout/GroundTile.cpp \
    ../src/osmscout/util/FileWriter.cpp \
    ../src/osmscout/SegmentAttributes.cpp \
    ../src/osmscout/ObjectRef.cpp \
    ../src/osmscout/Progress.cpp \
    ../src/osmscout/ost/ParserT.cpp \
    ../src/osmscout/ost/ScannerT.cpp \
    ../src/osmscout/MapPainter.cpp \
    ../src/osmscout/Partitionmodel.cpp \
    ../src/osmscout/Partitioning.cpp \
    ../src/osmscout/Routing.cpp \
    ../../PiLibocik/src/preparedata.cpp \
    ../../PiLibocik/src/position.cpp \
    ../../PiLibocik/src/poi.cpp \
    ../../PiLibocik/src/pilibocik.cpp \
    ../../PiLibocik/src/poifileppoi.cpp \
    ../../PiLibocik/src/boundarybox.cpp \
    ../../PiLibocik/src/geohash.cpp \
    ../../PiLibocik/src/partition/boundaryedge.cpp \
    ../../PiLibocik/src/partition/edge.cpp \
    ../../PiLibocik/src/partition/node.cpp \
    ../../PiLibocik/src/partition/partitionfile.cpp \
    ../../PiLibocik/src/partition/routenode.cpp \
    ../../PiLibocik/src/partition/restriction.cpp \
    ../../PiLibocik/src/partition/way.cpp

INCLUDEPATH += ./../include/ \
    ../../PiLibocik/include/

FORMS += \
    mainwindow.ui

RESOURCES += \
    icons.qrc

DEFINES += PiLibocik_WRITE_MODE









