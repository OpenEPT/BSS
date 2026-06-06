#ifndef PLOT_H
#define PLOT_H

#include <QWidget>
#include <QFont>
#include <QPushButton>
#include <QPair>
#include "Chart/qcustomplot.h"

// ── Line widths ───────────────────────────────────────────────────────────
#define PLOT_LINE_WIDTH             3
#define PLOT_AXIS_WIDTH             2
#define PLOT_AXIS_TICK_LENGTH       3

// ── Axis tick/label font (brojevi na osama: 0, 500, 1000...) ─────────────
#define PLOT_TICK_LABEL_FONT_SIZE   14
#define PLOT_TICK_LABEL_FONT_WEIGHT QFont::Normal
#define PLOT_TICK_LABEL_FONT_FAMILY "Timew New Roman"
#define PLOT_TICK_LABEL_FONT \
QFont(PLOT_TICK_LABEL_FONT_FAMILY, PLOT_TICK_LABEL_FONT_SIZE, PLOT_TICK_LABEL_FONT_WEIGHT)

// ── Axis label font (jedinice: [s], [V], [A]) ────────────────────────────
#define PLOT_AXIS_LABEL_FONT_SIZE   14
#define PLOT_AXIS_LABEL_FONT_WEIGHT QFont::Normal
#define PLOT_AXIS_LABEL_FONT_FAMILY "Timew New Roman"
#define PLOT_AXIS_LABEL_FONT \
    QFont(PLOT_AXIS_LABEL_FONT_FAMILY, PLOT_AXIS_LABEL_FONT_SIZE, PLOT_AXIS_LABEL_FONT_WEIGHT)

// ── Axis title font (naziv ose: Battery Current, SoC...) ─────────────────
#define PLOT_AXIS_TITLE_FONT_SIZE   14
#define PLOT_AXIS_TITLE_FONT_WEIGHT QFont::DemiBold
#define PLOT_AXIS_TITLE_FONT_FAMILY "Timew New Roman"
#define PLOT_AXIS_TITLE_FONT \
    QFont(PLOT_AXIS_TITLE_FONT_FAMILY, PLOT_AXIS_TITLE_FONT_SIZE, PLOT_AXIS_TITLE_FONT_WEIGHT)

// ── Title font (naslov grafika) ───────────────────────────────────────────
#define PLOT_TITLE_FONT_SIZE        14
#define PLOT_TITLE_FONT_WEIGHT      QFont::Bold
#define PLOT_TITLE_FONT_FAMILY      "Timew New Roman"
#define PLOT_TITLE_FONT \
    QFont(PLOT_TITLE_FONT_FAMILY, PLOT_TITLE_FONT_SIZE, PLOT_TITLE_FONT_WEIGHT)

// ── Legend font ───────────────────────────────────────────────────────────
#define LEGEND_FONT_SIZE            14
#define LEGEND_FONT_WEIGHT          QFont::DemiBold
#define LEGEND_FONT_FAMILY          "Timew New Roman"
#define LEGEND_FONT \
    QFont(LEGEND_FONT_FAMILY, LEGEND_FONT_SIZE, LEGEND_FONT_WEIGHT)

// ── Legend icon/padding sizes ─────────────────────────────────────────────
#define LEGEND_ICON_SIZE            14
#define LEGEND_ICON_TEXT_PADDING    10
#define LEGEND_PADDING              6
#define LEGEND_SPACING              4

    class Plot : public QWidget
{
    Q_OBJECT
public:
    explicit    Plot(int mw, int mh, bool aEnableTracking = true, QWidget *parent = nullptr, bool aEnableLegend = false);

    void        scatterAddGraph();
    void        scatterAddData(QVector<double> data, QVector<double> keys);
    void        scatterAddAllDataWithName(QVector<QPair<QString, int>> data);
    void        scatterAddDataWithName(double value, double keys, QString name);
    void        scatterReplotDataWithName();

    void        setData(QVector<double> data, QVector<double> keys);
    void        appendData(QVector<double> data, QVector<double> keys);
    void        setYRange(double min, double max);
    void        setYLabel(QString label);
    void        setXRange(double min, double max);
    void        setXLabel(QString label);
    void        setTitle(QString aTitle);
    void        setGraphName(int graphIndex, QString name);
    void        addLineGraph(QColor color, QString name = "");
    void        setSecondGraphData(QVector<double> data, QVector<double> keys);
    void        appendData2(QVector<double> data, QVector<double> keys);
    void        clear();

    void        setReplotActive(bool active);
    void        replotAll();
    void        replotOnly();
    void        clearAllGraphs();
    void        enableLegend(bool enable);

    void        addLineGraphWithStyle(QColor color, QString name, Qt::PenStyle style, int width = 1);
    void        setGraphLineStyle(int graphIndex, Qt::PenStyle style, QColor color, const QString &name);
    void        setGraphData(int graphIndex, QVector<double> data, QVector<double> keys);
    void        setGraphLineWidth(int graphIndex, int width);
    void        setAllGraphsInteractable(bool enable);

    int         graphCount() { return plot->graphCount(); }
    QCPGraph*   getGraph(int index) { return plot->graph(index); }
    void setGraphDataOnly(int graphIndex, QVector<double> data, QVector<double> keys);

    // ── Runtime font setteri ──────────────────────────────────────────────
    void        setAxisLabelFont(const QFont &font);   // brojevi na osama
    void        setAxisTitleFont(const QFont &font);   // naziv ose [s], [V]
    void        setTitleFont(const QFont &font);       // naslov grafika
    void        setLegendFont(const QFont &font);      // legenda

signals:
    void        sigScatterKeyAndName(QString name, double key);

private slots:
    void        onZoomIn();
    void        onZoomOut();
    void        onZoomExpand();
    void        onZoomArea();
    void        onMoveGraph();
    void        onTrackGraph();
    void        onSaveGraph();

private:
    QCustomPlot     *plot;

    QPushButton     *zoomIn;
    QPushButton     *zoomOut;
    QPushButton     *zoomExpand;
    QPushButton     *zoomArea;
    QPushButton     *moveGraph;
    QPushButton     *trackGraph;
    QPushButton     *saveGraph;

    QFont           *scatterFont;
    QCPTextElement  *title;

    QVector<double>     xData;
    QVector<double>     yData;
    QVector<double>     plotXData;
    QVector<double>     plotYData;
    QVector<double>     epDataKey;
    QVector<QString>    epDataName;
    QVector<QCPItemText *> textData;
    QVector<double>     xData2;
    QVector<double>     yData2;

    bool        enableTracking;
    bool        replotActive;
    bool        plot2Enabled;
    bool        scatterGraphAdded;

    void        setButtonStyle();
};

#endif // PLOT_H
