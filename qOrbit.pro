#-------------------------------------------------
#
# Project created by QtCreator 2014-08-13T17:54:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qOrbit
TEMPLATE = app

INCLUDEPATH += libsgp4

SOURCES += main.cpp \
    qOrbit.cpp \
    qPolarView.cpp \
	QSimpleSatelliteMap.cpp \
    libsgp4/CoordGeodetic.cpp \
    libsgp4/CoordTopocentric.cpp \
    libsgp4/DateTime.cpp \
    libsgp4/Eci.cpp \
    libsgp4/Globals.cpp \
    libsgp4/Observer.cpp \
    libsgp4/OrbitalElements.cpp \
    libsgp4/SGP4.cpp \
    libsgp4/SolarPosition.cpp \
    libsgp4/TimeSpan.cpp \
    libsgp4/Tle.cpp \
    libsgp4/Util.cpp \
    libsgp4/Vector.cpp \
    PassTable.cpp \
    PassDetails.cpp \
    PassCalculator.cpp

HEADERS  += qorbit.h \
    Footprint.h \
    QSimpleSatelliteMap.h \
    Satellite.h \
    ui_qorbit.h \
	qPolarView.h \
    libsgp4/CoordGeodetic.h \
    libsgp4/CoordTopocentric.h \
    libsgp4/DateTime.h \
    libsgp4/DecayedException.h \
    libsgp4/Eci.h \
    libsgp4/Globals.h \
    libsgp4/Observer.h \
    libsgp4/OrbitalElements.h \
    libsgp4/SatelliteException.h \
    libsgp4/SGP4.h \
    libsgp4/SolarPosition.h \
    libsgp4/TimeSpan.h \
    libsgp4/Tle.h \
    libsgp4/TleException.h \
    libsgp4/Util.h \
    libsgp4/Vector.h \
    qpredict_footprint.h \
    PassTable.h \
    PassDetails.h \
    PassCalculator.h

FORMS    += qorbit.ui
