#include "pagerender.h"
#include "sequentialpagewidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

SequentialPageWidget::SequentialPageWidget(QWidget *parent)
    : QWidget(parent)
    , m_pageCacheLimit(9)
    , m_PageRender(new PageRender())
    , m_pageSpacing(8)
    , m_pageIndex(0)
    , m_totalPages(0)
    , m_zoom(1.)
    , m_screenResolution(QGuiApplication::primaryScreen()->logicalDotsPerInch() / 72)
    , m_placeholderIcon(":/new/images/busy.png")
    , m_document(NULL)
{
  //  qDebug() << QGuiApplication::primaryScreen()->logicalDotsPerInch();
    connect(m_PageRender, SIGNAL(pageReady(int, qreal, QImage)), this, SLOT(pageLoaded(int, qreal, QImage)), Qt::QueuedConnection);
    grabGesture(Qt::SwipeGesture);
}

SequentialPageWidget::~SequentialPageWidget()
{
    delete m_PageRender;
}

bool SequentialPageWidget::setDocument(const QString &filePath)
{
    m_document = MuPDF::loadDocument(filePath);
    if (NULL == m_document)
    {
        return false;
    }

    m_PageRender->setDocument(m_document);
    m_totalPages = m_document->numPages();
    m_pageSizes.clear();

    for (int page = 0; page < m_document->numPages(); ++page)
    {
         m_pageSizes.append(m_document->page(page)->size()*m_screenResolution);
    }

    invalidate();
    return true;
}

int SequentialPageWidget::getPage()
{
    QRect rect = this->visibleRegion().boundingRect();

    int y1 = rect.y();
    int y2 = rect.y() + rect.height();

    int y = 0;
    int page = 0;
    for (page = 0; page < m_totalPages; ++page)
    {
        int avg = (y1+y2)/2;
        int size = m_pageSpacing + pageSize(page).toSize().height();
        if (y == y1)
        {
            m_pageIndex = page;
            break;
        }
        if (y < y1 && y+size >= y2)
        {
            m_pageIndex = page;
            break;
        }
        if (y > y1 && y+size <= y2)
        {
            m_pageIndex = page;
            break;
        }
        if (y <= avg && y+size > y2)
        {
            m_pageIndex = page;
            break;
        }
        if (y > avg && y+size > y2)
        {
            m_pageIndex = page-1;
            break;
        }
        y += size;
    }
    return m_pageIndex;
}

void SequentialPageWidget::nextPage()
{
    if (m_pageIndex < m_totalPages-1)
    {
        ++m_pageIndex;
    }
}

void SequentialPageWidget::previousPage()
{
    if (m_pageIndex > 0)
    {
        --m_pageIndex;
    }
}

void SequentialPageWidget::goToPage(int page)
{
    if (page >= 0 && page < m_totalPages)
    {
        m_pageIndex = page;
    }
}

void SequentialPageWidget::zoomIn()
{
    if (m_zoom < 10.0f)
    {
        m_zoom += 0.1f;
        invalidate();
    }
}

void SequentialPageWidget::zoomOut()
{
    if (m_zoom > 0.1f)
    {
        m_zoom -= 0.1f;
        invalidate();
    }
}

QSizeF SequentialPageWidget::pageSize(int page)
{
    return m_pageSizes.value(page) * m_zoom;
}

void SequentialPageWidget::invalidate()
{
    QSizeF totalSize(0, m_pageSpacing);
    QSizeF size(0, 0);
    for (int page = 0; page < m_totalPages; ++page)
    {
        size = pageSize(page);
        totalSize.setHeight(totalSize.height() + size.height() + m_pageSpacing);
        if (size.width() > totalSize.width())
        {
            totalSize.setWidth(size.width());
        }
    }
    totalSize += QSizeF(0.49,0.49);
    m_totalSize = totalSize.toSize();
    setMinimumSize(m_totalSize);
    m_pageCache.clear();
    update();
}

int SequentialPageWidget::yForPage()
{
    int y = 0;
    for (int page = 0; page < m_totalPages && page < m_pageIndex; ++page)
    {
        y += m_pageSpacing + pageSize(page).toSize().height();
    }
    return y;
}

QImage SequentialPageWidget::getPDFImage(int index)
{
    QImage img;
    if (index >= 0 && index < m_totalPages)
    {
        MuPDF::Page * objpage = m_document->page(index);
        img = objpage->renderImage();
    }
    return img;
}


void SequentialPageWidget::pageLoaded(int page, qreal zoom, QImage image)
{
    Q_UNUSED(zoom)
    if (m_cachedPagesLRU.length() > m_pageCacheLimit)
    {
        m_pageCache.remove(m_cachedPagesLRU.takeFirst());
    }
    m_pageCache.insert(page, image);
    m_cachedPagesLRU.append(page);
    update();
}

void SequentialPageWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);

    if (0 == m_totalPages)
    {
       return;
    }

    // Find the first page that needs to be rendered
    int page = 0;
    int y = 0;

    while (page < m_totalPages)
    {
        int height = pageSize(page).toSize().height();
        if (y + height >= event->rect().top())
        {
            break;
        }
        y += m_pageSpacing + height;
        ++page;
    }
    y += m_pageSpacing;

    // Actually render pages
    while (y < event->rect().bottom() && page < m_totalPages)
    {
        QSizeF size = pageSize(page);

        if (m_pageCache.contains(page))
        {
            const QImage &img = m_pageCache[page];
            painter.fillRect((width() - img.width()) / 2, y, size.width(), size.height(), Qt::white);
            painter.drawImage((width() - img.width()) / 2, y, img);
            getPage();
            emit updatePdfInfo(m_pageIndex, m_totalPages, m_zoom);
        }
        else
        {
            painter.fillRect((width() - size.width()) / 2, y, size.width(), size.height(), Qt::white);
            painter.drawPixmap((size.width() - m_placeholderIcon.width()) / 2,
                               (size.height() - m_placeholderIcon.height()) / 2, m_placeholderIcon);
            m_PageRender->requestPage(page, m_screenResolution * m_zoom);
        }
        y += size.height() + m_pageSpacing;
        ++page;
    }
}
