#include "pagerender.h"
#include "mupdfdocument.h"
#include "mupdfpage.h"
#include <QPainter>
#include <QUrl>

PageRender::PageRender(QObject *parent)
    : QThread(parent)
    , m_page(0)
    , m_zoom(1.)
    , m_document(NULL)
{
}

void PageRender::setDocument(MuPDF::Document* document)
{
    m_document = document;
}

void PageRender::requestPage(int page, qreal zoom, Priority priority)
{
    m_page = page;
    m_zoom = zoom;
    start(priority);
}

void PageRender::run()
{
    renderPage(m_page, m_zoom);
}

void PageRender::renderPage(int page, qreal zoom)
{
    if (!m_document)
    {
        return;
    }

    MuPDF::Page* objpage = m_document->page(page);
    const QImage &img = objpage->renderImage(zoom, zoom);
    emit pageReady(page, zoom, img);
}


