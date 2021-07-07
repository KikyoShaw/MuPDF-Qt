#ifndef PAGERENDER_H
#define PAGERENDER_H

#include <QImage>
#include <QThread>
#include "mupdfdocument.h"
#include "mupdfpage.h"

class PageRender : public QThread
{
    Q_OBJECT

public:
    explicit PageRender(QObject *parent = NULL);

signals:
    void pageReady(int page, qreal zoom, QImage image);

public slots:
    void setDocument(MuPDF::Document* document);
    void requestPage(int page, qreal zoom, Priority priority = QThread::NormalPriority);

protected:
    void run();

private:
    void renderPage(int page, qreal zoom);

private:
    int   m_page;
    qreal m_zoom;
    MuPDF::Document *m_document;
};

#endif // PAGERENDER_H
