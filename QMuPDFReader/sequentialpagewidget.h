#ifndef SEQUENTIALPAGEWIDGET_H
#define SEQUENTIALPAGEWIDGET_H

#include <QWidget>
#include "mupdfdocument.h"
#include "mupdfpage.h"

class PageRender;

class SequentialPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequentialPageWidget(QWidget *parent = 0);
    ~SequentialPageWidget();

    void paintEvent(QPaintEvent * event);
    bool setDocument(const QString &filePath);
    int getPage();
    int yForPage();

    QImage getPDFImage(int index);

signals:
    void updatePdfInfo(int pageIndex, int totalPages, qreal zoom);

public slots:
    void nextPage();
    void previousPage();
    void goToPage(int page);
    void zoomIn();
    void zoomOut();

private slots:
    void pageLoaded(int page, qreal zoom, QImage image);

private:
    void invalidate();
    QSizeF pageSize(int page);

private:
    QHash<int, QImage> m_pageCache;
    QVector<int> m_cachedPagesLRU;
    int m_pageCacheLimit;
    QVector<QSizeF> m_pageSizes;
    PageRender *m_PageRender;

    int m_pageSpacing;
    int m_pageIndex;
    int m_totalPages;
    QSize m_totalSize;
    qreal m_zoom;
    qreal m_screenResolution;
    QPixmap m_placeholderIcon;

    MuPDF::Document *m_document;
};

#endif // SEQUENTIALPAGEWIDGET_H
