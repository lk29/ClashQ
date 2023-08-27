QT += core gui widgets network printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Application.cpp \
    ConnectionModel.cpp \
    ConnectionPage.cpp \
    ConnectionSortModel.cpp \
    LogPage.cpp \
    QCustomPlot/src/axis/axis.cpp \
    QCustomPlot/src/axis/axisticker.cpp \
    QCustomPlot/src/axis/axistickerdatetime.cpp \
    QCustomPlot/src/axis/axistickerfixed.cpp \
    QCustomPlot/src/axis/axistickerlog.cpp \
    QCustomPlot/src/axis/axistickerpi.cpp \
    QCustomPlot/src/axis/axistickertext.cpp \
    QCustomPlot/src/axis/axistickertime.cpp \
    QCustomPlot/src/axis/labelpainter.cpp \
    QCustomPlot/src/axis/range.cpp \
    QCustomPlot/src/colorgradient.cpp \
    QCustomPlot/src/core.cpp \
    QCustomPlot/src/item.cpp \
    QCustomPlot/src/items/item-bracket.cpp \
    QCustomPlot/src/items/item-curve.cpp \
    QCustomPlot/src/items/item-ellipse.cpp \
    QCustomPlot/src/items/item-line.cpp \
    QCustomPlot/src/items/item-pixmap.cpp \
    QCustomPlot/src/items/item-rect.cpp \
    QCustomPlot/src/items/item-straightline.cpp \
    QCustomPlot/src/items/item-text.cpp \
    QCustomPlot/src/items/item-tracer.cpp \
    QCustomPlot/src/layer.cpp \
    QCustomPlot/src/layout.cpp \
    QCustomPlot/src/layoutelements/layoutelement-axisrect.cpp \
    QCustomPlot/src/layoutelements/layoutelement-colorscale.cpp \
    QCustomPlot/src/layoutelements/layoutelement-legend.cpp \
    QCustomPlot/src/layoutelements/layoutelement-textelement.cpp \
    QCustomPlot/src/lineending.cpp \
    QCustomPlot/src/paintbuffer.cpp \
    QCustomPlot/src/painter.cpp \
    QCustomPlot/src/plottable.cpp \
    QCustomPlot/src/plottables/plottable-bars.cpp \
    QCustomPlot/src/plottables/plottable-colormap.cpp \
    QCustomPlot/src/plottables/plottable-curve.cpp \
    QCustomPlot/src/plottables/plottable-errorbar.cpp \
    QCustomPlot/src/plottables/plottable-financial.cpp \
    QCustomPlot/src/plottables/plottable-graph.cpp \
    QCustomPlot/src/plottables/plottable-statisticalbox.cpp \
    QCustomPlot/src/polar/layoutelement-angularaxis.cpp \
    QCustomPlot/src/polar/polargraph.cpp \
    QCustomPlot/src/polar/polargrid.cpp \
    QCustomPlot/src/polar/radialaxis.cpp \
    QCustomPlot/src/scatterstyle.cpp \
    QCustomPlot/src/selection.cpp \
    QCustomPlot/src/selectiondecorator-bracket.cpp \
    QCustomPlot/src/selectionrect.cpp \
    QCustomPlot/src/vector2d.cpp \
    TrafficPage.cpp \
    TrafficSpeedTicker.cpp \
    Utils.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    Application.h \
    ConnectionModel.h \
    ConnectionPage.h \
    ConnectionSortModel.h \
    LogPage.h \
    MainWindow.h \
    QCustomPlot/src/axis/axis.h \
    QCustomPlot/src/axis/axisticker.h \
    QCustomPlot/src/axis/axistickerdatetime.h \
    QCustomPlot/src/axis/axistickerfixed.h \
    QCustomPlot/src/axis/axistickerlog.h \
    QCustomPlot/src/axis/axistickerpi.h \
    QCustomPlot/src/axis/axistickertext.h \
    QCustomPlot/src/axis/axistickertime.h \
    QCustomPlot/src/axis/labelpainter.h \
    QCustomPlot/src/axis/range.h \
    QCustomPlot/src/colorgradient.h \
    QCustomPlot/src/core.h \
    QCustomPlot/src/datacontainer.h \
    QCustomPlot/src/global.h \
    QCustomPlot/src/item.h \
    QCustomPlot/src/items/item-bracket.h \
    QCustomPlot/src/items/item-curve.h \
    QCustomPlot/src/items/item-ellipse.h \
    QCustomPlot/src/items/item-line.h \
    QCustomPlot/src/items/item-pixmap.h \
    QCustomPlot/src/items/item-rect.h \
    QCustomPlot/src/items/item-straightline.h \
    QCustomPlot/src/items/item-text.h \
    QCustomPlot/src/items/item-tracer.h \
    QCustomPlot/src/layer.h \
    QCustomPlot/src/layout.h \
    QCustomPlot/src/layoutelements/layoutelement-axisrect.h \
    QCustomPlot/src/layoutelements/layoutelement-colorscale.h \
    QCustomPlot/src/layoutelements/layoutelement-legend.h \
    QCustomPlot/src/layoutelements/layoutelement-textelement.h \
    QCustomPlot/src/lineending.h \
    QCustomPlot/src/paintbuffer.h \
    QCustomPlot/src/painter.h \
    QCustomPlot/src/plottable.h \
    QCustomPlot/src/plottable1d.h \
    QCustomPlot/src/plottables/plottable-bars.h \
    QCustomPlot/src/plottables/plottable-colormap.h \
    QCustomPlot/src/plottables/plottable-curve.h \
    QCustomPlot/src/plottables/plottable-errorbar.h \
    QCustomPlot/src/plottables/plottable-financial.h \
    QCustomPlot/src/plottables/plottable-graph.h \
    QCustomPlot/src/plottables/plottable-statisticalbox.h \
    QCustomPlot/src/polar/layoutelement-angularaxis.h \
    QCustomPlot/src/polar/polargraph.h \
    QCustomPlot/src/polar/polargrid.h \
    QCustomPlot/src/polar/radialaxis.h \
    QCustomPlot/src/qcp.h \
    QCustomPlot/src/scatterstyle.h \
    QCustomPlot/src/selection.h \
    QCustomPlot/src/selectiondecorator-bracket.h \
    QCustomPlot/src/selectionrect.h \
    QCustomPlot/src/vector2d.h \
    TrafficPage.h \
    TrafficSpeedTicker.h \
    Utils.h

FORMS += \
    ConnectionPage.ui \
    LogPage.ui \
    MainWindow.ui \
    TrafficPage.ui

INCLUDEPATH += $$[QT_INSTALL_PREFIX]/../../Tools/OpenSSL/Win_x64/include QCustomPlot/src

LIBS += -L$$[QT_INSTALL_PREFIX]/../../Tools/OpenSSL/Win_x64/lib -llibcrypto -lAdvapi32

RC_ICONS = app.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ClashQ.qrc
