QT       += core gui

TARGET = build/example-build
TEMPLATE = app

HEADERS += \
    mainwindow.h \
    ../include/osmscout/Database.h \
    ../include/osmscout/StyleConfigLoader.h \
    ../include/osmscout/Util.h \
    ../include/osmscout/MapPainterQt.h \
    ../include/osmscout/MapPainterQt.h \
    ../include/osmscout/util/Geometry.h \
    ../include/osmscout/private/Math.h \
    ../include/osmscout/StyleConfig.h \
    ../include/osmscout/TypeConfig.h \
    ../include/osmscout/NodeDataFile.h \
    ../include/osmscout/RelationDataFile.h \
    ../include/osmscout/WayDataFile.h \
    ../include/osmscout/OptimizeLowZoom.h \
    ../include/osmscout/NodeIndex.h \
    ../include/osmscout/WayIndex.h \
    ../include/osmscout/AreaAreaIndex.h \
    ../include/osmscout/AreaNodeIndex.h \
    ../include/osmscout/AreaWayIndex.h \
    ../include/osmscout/CityStreetIndex.h \
    ../include/osmscout/NodeUseIndex.h \
    ../include/osmscout/WaterIndex.h \
    ../include/osmscout/Route.h \
    ../include/osmscout/util/Cache.h \
    ../include/osmscout/RoutingProfile.h \
    ../include/osmscout/TypeConfigLoader.h \
    ../include/osmscout/util/StopClock.h \
    ../include/osmscout/oss/Parser.h \
    ../include/osmscout/oss/Scanner.h \
    ../include/osmscout/system/Types.h \
    ../include/osmscout/MapPainter.h \
    ../include/osmscout/Point.h \
    ../include/osmscout/util/Projection.h \
    ../include/osmscout/private/Config.h \
    ../include/osmscout/Types.h \
    ../include/osmscout/Tag.h \
    ../include/osmscout/util/Reference.h \
    ../include/osmscout/Node.h \
    ../include/osmscout/DataFile.h \
    ../include/osmscout/Relation.h \
    ../include/osmscout/Way.h \
    ../include/osmscout/util/FileScanner.h \
    ../include/osmscout/util/String.h \
    ../include/osmscout/util/Transformation.h \
    ../include/osmscout/NumericIndex.h \
    ../include/osmscout/AdminRegion.h \
    ../include/osmscout/Location.h \
    ../include/osmscout/GroundTile.h \
    ../include/osmscout/CoreFeatures.h \
    ../include/osmscout/MapFeatures.h \
    ../include/osmscout/util/FileWriter.h \
    ../include/osmscout/SegmentAttributes.h \
    ../include/osmscout/ObjectRef.h \
    ../include/osmscout/Progress.h \
    ../include/osmscout/ost/ParserT.h \
    ../include/osmscout/ost/ScannerT.h

SOURCES += \
    mappainterqt.cpp \
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
    ../src/osmscout/MapPainter.cpp

INCLUDEPATH += ./../include/

FORMS += \
    mainwindow.ui

RESOURCES += \
    icons.qrc


