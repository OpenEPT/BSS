#include <QtOpenGL>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include "plot.h"

#define BUTTONS_SIZE 30

Plot::Plot(int mw, int mh, bool aEnableTracking, QWidget *parent, bool aEnableLegend)
    : QWidget{parent}
{
    this->setParent(parent);

    // ── QCustomPlot ───────────────────────────────────────────────────────
    plot = new QCustomPlot();
    plot->setMinimumSize(mw, mh);
    plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    plot->setAntialiasedElements(QCP::aeAll);

    plot->addGraph();
    QPen defaultPen(QColor(40, 110, 255));
    defaultPen.setWidth(PLOT_LINE_WIDTH);
    plot->graph(0)->setPen(defaultPen);

    plot->setInteraction(QCP::iSelectPlottables, true);
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);

    // ── Dugmad ────────────────────────────────────────────────────────────
    zoomIn    = new QPushButton();
    zoomOut   = new QPushButton();
    zoomExpand= new QPushButton();
    zoomArea  = new QPushButton();
    moveGraph = new QPushButton();
    trackGraph= new QPushButton();
    saveGraph = new QPushButton();

    auto setupBtn = [](QPushButton *btn, const QString &iconPath,
                       const QString &tip, int size = BUTTONS_SIZE) {
        QPixmap px(iconPath);
        if (!px.isNull()) {
            btn->setIcon(QIcon(px));
            btn->setIconSize(QSize(15, 15));
        }
        btn->setToolTip(tip);
        btn->setFixedSize(size, size);
    };

    setupBtn(zoomIn,    ":/images/NewSet/zoom_in.png",       "Zoom in");
    setupBtn(zoomOut,   ":/images/NewSet/zoom_out.png",      "Zoom out");
    setupBtn(zoomExpand,":/images/NewSet/expand.png",        "Fit to full data");
    setupBtn(zoomArea,  ":/images/NewSet/zoom_area.png",     "Zoom area");
    setupBtn(moveGraph, ":/images/NewSet/moveGraph.png",     "Move graph");
    setupBtn(trackGraph,":/images/NewSet/tracking_graph.png","Enable graph tracking");
    setupBtn(saveGraph, ":/images/NewSet/save.png",          "Save graph as image");

    // ── Layout ────────────────────────────────────────────────────────────
    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->addWidget(zoomIn);
    buttonsLayout->addWidget(zoomOut);
    buttonsLayout->addWidget(zoomExpand);
    buttonsLayout->addWidget(zoomArea);
    buttonsLayout->addWidget(moveGraph);
    buttonsLayout->addWidget(trackGraph);
    buttonsLayout->addWidget(saveGraph);
    buttonsLayout->setAlignment(Qt::AlignCenter);

    QHBoxLayout *plotLayout = new QHBoxLayout(this);
    plotLayout->addLayout(buttonsLayout);
    plotLayout->addWidget(plot);
    plotLayout->setAlignment(Qt::AlignLeft);
    plotLayout->setSpacing(3);

    // ── Naslov grafika ────────────────────────────────────────────────────
    plot->plotLayout()->insertRow(0);
    title = new QCPTextElement(plot, "NN", PLOT_TITLE_FONT);
    plot->plotLayout()->addElement(0, 0, title);

    // ── Ose ───────────────────────────────────────────────────────────────
    QPen axisPen(Qt::black, PLOT_AXIS_WIDTH);
    QPen subTickPen(Qt::black, PLOT_AXIS_WIDTH - 1);

    plot->xAxis->setBasePen(axisPen);
    plot->yAxis->setBasePen(axisPen);
    plot->xAxis->setTickPen(axisPen);
    plot->yAxis->setTickPen(axisPen);
    plot->xAxis->setSubTickPen(subTickPen);
    plot->yAxis->setSubTickPen(subTickPen);
    plot->xAxis->setTickLength(PLOT_AXIS_TICK_LENGTH);
    plot->yAxis->setTickLength(PLOT_AXIS_TICK_LENGTH);

    // Gornja i desna osa vidljive, bez labela
    plot->xAxis2->setVisible(true);
    plot->yAxis2->setVisible(true);
    plot->xAxis2->setBasePen(axisPen);
    plot->yAxis2->setBasePen(axisPen);
    plot->xAxis2->setTickPen(axisPen);
    plot->yAxis2->setTickPen(axisPen);
    plot->xAxis2->setTickLabels(false);
    plot->yAxis2->setTickLabels(false);

    // ── Fontovi osa ───────────────────────────────────────────────────────
    plot->xAxis->setTickLabelFont(PLOT_TICK_LABEL_FONT);   // brojevi: 0, 500...
    plot->yAxis->setTickLabelFont(PLOT_TICK_LABEL_FONT);
    plot->xAxis->setLabelFont(PLOT_AXIS_TITLE_FONT);       // naziv: [s], [V]
    plot->yAxis->setLabelFont(PLOT_AXIS_TITLE_FONT);

    // ── Legenda ───────────────────────────────────────────────────────────
    plot->legend->setFont(LEGEND_FONT);
    plot->legend->setIconSize(LEGEND_ICON_SIZE, LEGEND_ICON_SIZE);
    plot->legend->setIconTextPadding(LEGEND_ICON_TEXT_PADDING);
    plot->legend->setBorderPen(QPen(Qt::gray, 1));
    plot->legend->setBrush(QBrush(QColor(255, 255, 255, 220)));
    plot->legend->setFillOrder(QCPLegend::foRowsFirst);

    // ── Pozicija legende — dole levo ──────────────────────────────────────────
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignLeft);

    // ── Init flags ────────────────────────────────────────────────────────
    enableTracking    = aEnableTracking;
    replotActive      = true;
    scatterGraphAdded = false;
    plot2Enabled      = false;

    scatterFont = new QFont("Times", 14);
    scatterFont->setBold(true);

    if (aEnableLegend) {
        plot2Enabled = true;
        plot->legend->setVisible(true);
    }

    // ── Connections ───────────────────────────────────────────────────────
    connect(zoomIn,    SIGNAL(pressed()), this, SLOT(onZoomIn()));
    connect(zoomOut,   SIGNAL(pressed()), this, SLOT(onZoomOut()));
    connect(zoomExpand,SIGNAL(pressed()), this, SLOT(onZoomExpand()));
    connect(zoomArea,  SIGNAL(pressed()), this, SLOT(onZoomArea()));
    connect(moveGraph, SIGNAL(pressed()), this, SLOT(onMoveGraph()));
    connect(trackGraph,SIGNAL(pressed()), this, SLOT(onTrackGraph()));
    connect(saveGraph, SIGNAL(pressed()), this, SLOT(onSaveGraph()));

    setButtonStyle();
}

// ── Scatter ───────────────────────────────────────────────────────────────

void Plot::scatterAddGraph()
{
    scatterGraphAdded = true;
    plot->addGraph();
    plot->graph(1)->setLineStyle(QCPGraph::lsNone);
    plot->graph(1)->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, 10));
}

void Plot::scatterAddData(QVector<double> data, QVector<double> keys)
{
    plot->graph(1)->setData(keys, data);
    for (int i = 0; i < data.size(); i++) {
        QCPItemText *lbl = new QCPItemText(plot);
        lbl->setPositionAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        lbl->position->setType(QCPItemPosition::ptPlotCoords);
        lbl->position->setCoords(keys[i], data[i] + 0.5);
        lbl->setText(QString::number(i));
        lbl->setFont(QFont("Times", 14));
        lbl->setColor(Qt::red);
    }
    plot->replot();
}

void Plot::scatterAddAllDataWithName(QVector<QPair<QString, int>> data)
{
    for (int i = 0; i < data.size(); i++) {
        if (data[i].second >= xData.size()) break;
        plot->graph(1)->addData(xData[data[i].second], yData[data[i].second]);
        QCPItemText *lbl = new QCPItemText(plot);
        lbl->setPositionAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        lbl->position->setType(QCPItemPosition::ptPlotCoords);
        lbl->position->setCoords(xData[data[i].second], yData[data[i].second]);
        lbl->setText(data[i].first);
        lbl->setFont(*scatterFont);
        lbl->setColor(Qt::black);
        textData.push_back(lbl);
    }
    plot->replot();
}

void Plot::scatterAddDataWithName(double /*value*/, double keys, QString name)
{
    if (keys >= xData.size() || keys >= yData.size()) {
        qDebug() << "Corresponding data not arrived";
        epDataKey.append(keys);
        epDataName.append(name);
        return;
    }
    plot->graph(1)->addData(xData[keys], yData[keys]);
    QCPItemText *lbl = new QCPItemText(plot);
    lbl->setPositionAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    lbl->position->setType(QCPItemPosition::ptPlotCoords);
    lbl->position->setCoords(xData[keys], yData[keys]);
    lbl->setText(name);
    lbl->setFont(QFont("Times", 14));
    lbl->setColor(Qt::red);
    textData.push_back(lbl);
    plot->yAxis->rescale(true);
    plot->replot();
}

void Plot::scatterReplotDataWithName()
{
    for (int i = 0; i < epDataKey.size(); i++) {
        if (epDataKey[i] > xData.size() || epDataKey[i] > yData.size()) break;
        int key = epDataKey[i];
        plot->graph(1)->addData(xData[key], yData[key]);
        QCPItemText *lbl = new QCPItemText(plot);
        lbl->setPositionAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        lbl->position->setType(QCPItemPosition::ptPlotCoords);
        lbl->position->setCoords(xData[key], yData[key]);
        lbl->setText(epDataName[i]);
        lbl->setFont(*scatterFont);
        lbl->setColor(Qt::red);
        textData.push_back(lbl);
        plot->yAxis->rescale(true);
        plot->replot();
        epDataKey.removeAt(i);
        epDataName.removeAt(i);
    }
}

// ── Data setters ──────────────────────────────────────────────────────────

void Plot::setData(QVector<double> data, QVector<double> keys)
{
    xData = keys;
    yData = data;
    if (replotActive) {
        plot->graph(0)->setData(xData, yData, true);
        plot->rescaleAxes(true);
        plot->replot();
    }
}

void Plot::appendData(QVector<double> data, QVector<double> keys)
{
    xData.append(keys);
    yData.append(data);
    plotXData.append(keys);
    plotYData.append(data);
    if (plotXData.at(plotXData.size() - 1) > 10000) {
        plotXData.remove(0, data.size());
        plotYData.remove(0, data.size());
    }
    if (replotActive) {
        plot->graph(0)->setData(plotXData, plotYData, true);
        plot->yAxis->rescale(true);
        plot->xAxis->rescale(true);
        plot->replot();
        scatterReplotDataWithName();
    }
}

void Plot::setSecondGraphData(QVector<double> data, QVector<double> keys)
{
    if (plot->graphCount() < 2) return;
    xData2 = keys;
    yData2 = data;
    plot->graph(1)->setData(keys, data, true);
}

void Plot::appendData2(QVector<double> data, QVector<double> keys)
{
    if (plot->graphCount() < 2) return;
    xData2.append(keys);
    yData2.append(data);
    plot->graph(1)->addData(keys, data);
    plot->yAxis->rescale(true);
    plot->xAxis->rescale(true);
    plot->replot();
}

void Plot::setGraphData(int graphIndex, QVector<double> data, QVector<double> keys)
{
    if (graphIndex >= plot->graphCount()) return;
    plot->graph(graphIndex)->setData(keys, data, true);
    plot->rescaleAxes(true);
    plot->replot();
}

// ── Axis / title setters ──────────────────────────────────────────────────

void Plot::setYRange(double min, double max) { plot->yAxis->setRange(min, max); plot->replot(); }
void Plot::setYLabel(QString label)          { plot->yAxis->setLabel(label);    plot->replot(); }
void Plot::setXRange(double min, double max) { plot->xAxis->setRange(min, max); plot->replot(); }
void Plot::setXLabel(QString label)          { plot->xAxis->setLabel(label);    plot->replot(); }

void Plot::setTitle(QString aTitle)
{
    title->setText(aTitle);
    plot->replot();
}

void Plot::setGraphName(int graphIndex, QString name)
{
    if (graphIndex < plot->graphCount())
        plot->graph(graphIndex)->setName(name);
}

// ── Font setteri (runtime) ────────────────────────────────────────────────

void Plot::setAxisLabelFont(const QFont &font)
{
    plot->xAxis->setTickLabelFont(font);
    plot->yAxis->setTickLabelFont(font);
    plot->replot();
}

void Plot::setAxisTitleFont(const QFont &font)
{
    plot->xAxis->setLabelFont(font);
    plot->yAxis->setLabelFont(font);
    plot->replot();
}

void Plot::setTitleFont(const QFont &font)
{
    title->setFont(font);
    plot->replot();
}

void Plot::setLegendFont(const QFont &font)
{
    plot->legend->setFont(font);
    plot->replot();
}

// ── Graph style ───────────────────────────────────────────────────────────

void Plot::addLineGraph(QColor color, QString name)
{
    plot->addGraph();
    int idx = plot->graphCount() - 1;
    QPen pen(color);
    pen.setWidth(PLOT_LINE_WIDTH);
    plot->graph(idx)->setPen(pen);
    if (!name.isEmpty())
        plot->graph(idx)->setName(name);
    plot->legend->setFont(LEGEND_FONT);
}

void Plot::addLineGraphWithStyle(QColor color, QString name, Qt::PenStyle style, int width)
{
    plot->addGraph();
    int idx = plot->graphCount() - 1;
    QPen pen(color);
    pen.setStyle(style);
    pen.setWidth(width > 0 ? width : PLOT_LINE_WIDTH);
    plot->graph(idx)->setPen(pen);
    if (!name.isEmpty())
        plot->graph(idx)->setName(name);
    if (plot->legend) {
        plot->legend->setVisible(true);
        plot->legend->setFont(LEGEND_FONT);
    }
}

void Plot::setGraphLineStyle(int graphIndex, Qt::PenStyle style, QColor color, const QString &name)
{
    if (graphIndex >= plot->graphCount()) return;
    QPen pen(color);
    pen.setStyle(style);
    pen.setWidth(PLOT_LINE_WIDTH);
    plot->graph(graphIndex)->setPen(pen);
    if (!name.isEmpty())
        plot->graph(graphIndex)->setName(name);
    plot->legend->setFont(LEGEND_FONT);
}

void Plot::setGraphLineWidth(int graphIndex, int width)
{
    if (graphIndex >= plot->graphCount()) return;
    QPen pen = plot->graph(graphIndex)->pen();
    pen.setWidth(width);
    plot->graph(graphIndex)->setPen(pen);
}

void Plot::setAllGraphsInteractable(bool enable)
{
    plot->legend->setVisible(true);
    for (int i = 0; i < plot->graphCount(); i++)
        plot->graph(i)->setSelectable(QCP::stWhole);

    if (enable) {
        disconnect(plot, &QCustomPlot::legendClick, nullptr, nullptr);
        connect(plot, &QCustomPlot::legendClick, this,
                [=](QCPLegend *, QCPAbstractLegendItem *item, QMouseEvent *) {
                    QCPPlottableLegendItem *pItem =
                        qobject_cast<QCPPlottableLegendItem *>(item);
                    if (pItem) {
                        pItem->plottable()->setVisible(!pItem->plottable()->visible());
                        pItem->setTextColor(pItem->plottable()->visible()
                                                ? QColor(0, 0, 0)
                                                : QColor(180, 180, 180));
                        plot->replot();
                    }
                });
    }
}

// ── Legend ────────────────────────────────────────────────────────────────

void Plot::enableLegend(bool enable)
{
    plot->legend->setVisible(enable);
    plot2Enabled = enable;
    plot->replot();
}

// ── Replot ────────────────────────────────────────────────────────────────

void Plot::replotAll()
{
    if (!xData.isEmpty())
        plot->graph(0)->setData(xData, yData, true);
    plot->rescaleAxes(true);
    plot->replot();
}

void Plot::setGraphDataOnly(int graphIndex, QVector<double> data, QVector<double> keys)
{
    if (graphIndex >= plot->graphCount()) return;
    plot->graph(graphIndex)->setData(keys, data, true);
    // bez rescale, bez replot
}

void Plot::replotOnly()
{
    plot->rescaleAxes(true);
    plot->replot();
}

void Plot::clearAllGraphs()
{
    while (plot->graphCount() > 0)
        plot->removeGraph(0);

    plot->addGraph();
    QPen defaultPen(QColor(40, 110, 255));
    defaultPen.setWidth(PLOT_LINE_WIDTH);
    plot->graph(0)->setPen(defaultPen);

    xData.clear();  yData.clear();
    xData2.clear(); yData2.clear();
    plot->replot();
}

void Plot::clear()
{
    if (plot->graphCount() > 0)
        plot->graph(0)->data()->clear();

    if (plot2Enabled && plot->graphCount() > 1) {
        plot->graph(1)->data()->clear();
        for (int i = 0; i < textData.size(); i++)
            plot->removeItem(textData[i]);
        textData.clear();
    }

    xData.clear();    yData.clear();
    xData2.clear();   yData2.clear();
    epDataKey.clear(); epDataName.clear();
    plotXData.clear(); plotYData.clear();

    plot->xAxis->setRange(0, 1000);
    plot->replot();
}

void Plot::setReplotActive(bool active)
{
    replotActive = active;
}

// ── Zoom / interaction slots ──────────────────────────────────────────────

void Plot::onZoomIn()
{
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);
    plot->xAxis->scaleRange(.85, plot->xAxis->range().center());
    plot->yAxis->scaleRange(.85, plot->yAxis->range().center());
    plot->replot();
    setButtonStyle();
}

void Plot::onZoomOut()
{
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);
    plot->xAxis->scaleRange(1.25, plot->xAxis->range().center());
    plot->yAxis->scaleRange(1.25, plot->yAxis->range().center());
    plot->replot();
    setButtonStyle();
}

void Plot::onZoomExpand()
{
    plot->graph(0)->setData(xData, yData, true);
    plot->setInteraction(QCP::iRangeDrag, false);
    plot->setInteraction(QCP::iRangeZoom, false);
    plot->rescaleAxes(true);
    plot->setSelectionRectMode(QCP::srmZoom);
    plot->replot();
    setButtonStyle();
}

void Plot::onZoomArea()
{
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);
    plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    plot->setSelectionRectMode(QCP::srmZoom);
    plot->replot();
    setButtonStyle();
}

void Plot::onMoveGraph()
{
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, false);
    plot->setInteraction(QCP::iSelectPlottables, true);
    plot->setSelectionRectMode(QCP::srmNone);
    plot->replot();
    setButtonStyle();
}

void Plot::onTrackGraph()
{
    enableTracking = !enableTracking;
    replotActive   = enableTracking;
    setButtonStyle();
}

void Plot::setButtonStyle()
{
    if (enableTracking) {
        trackGraph->setStyleSheet("background-color: rgb(255,197,172);");
        zoomIn->setEnabled(false);
        zoomOut->setEnabled(false);
        moveGraph->setEnabled(false);
        zoomExpand->setEnabled(false);
        zoomArea->setEnabled(false);
    } else {
        trackGraph->setStyleSheet("background-color: rgb(255,255,255);");
        zoomIn->setEnabled(true);
        zoomOut->setEnabled(true);
        moveGraph->setEnabled(true);
        zoomExpand->setEnabled(true);
        zoomArea->setEnabled(true);
    }
}

// ── Save ──────────────────────────────────────────────────────────────────

void Plot::onSaveGraph()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Graph",
        QDir::homePath() + "/graph.pdf",
        "PDF Document (*.pdf);;PNG Image (*.png);;JPEG Image (*.jpg)");

    if (filePath.isEmpty()) return;

    // Deblje ose za export
    QPen thickPen(Qt::black, PLOT_AXIS_WIDTH);
    QPen oldXBase = plot->xAxis->basePen();
    QPen oldYBase = plot->yAxis->basePen();
    QPen oldXTick = plot->xAxis->tickPen();
    QPen oldYTick = plot->yAxis->tickPen();

    plot->xAxis->setBasePen(thickPen);
    plot->yAxis->setBasePen(thickPen);
    plot->xAxis->setTickPen(thickPen);
    plot->yAxis->setTickPen(thickPen);
    plot->replot();

    if (filePath.endsWith(".pdf", Qt::CaseInsensitive)) {
        plot->savePdf(filePath, 600, 400, QCP::epNoCosmetic, "OpenEPT", title->text());
    } else if (filePath.endsWith(".jpg", Qt::CaseInsensitive) ||
               filePath.endsWith(".jpeg", Qt::CaseInsensitive)) {
        plot->saveJpg(filePath, 960, 540, 1.0, 90);
    } else {
        if (!filePath.endsWith(".png", Qt::CaseInsensitive))
            filePath += ".png";
        plot->savePng(filePath, 1920, 1080, 1.0, -1);
    }

    // Vrati originalne penove
    plot->xAxis->setBasePen(oldXBase);
    plot->yAxis->setBasePen(oldYBase);
    plot->xAxis->setTickPen(oldXTick);
    plot->yAxis->setTickPen(oldYTick);
    plot->replot();
}
