#ifndef MUPDF_PAGE_H
#define MUPDF_PAGE_H

#include <QList>

class QImage;
class QString;
class QPointF;
class QSizeF;
class QRectF;

namespace MuPDF
{
class Page;
class PagePrivate;
class Document;

/**
 * @brief A page.
 *
 * @note When you are doing something with this page, make sure the Document
 * who generate this page is valid.
 */
class Page
{
public:
    ~Page();
    bool isValid() const;
    QImage renderImage(float scaleX = 1.0f, float scaleY = 1.0f, float rotation = 0.0f) const;
    QSizeF size() const;
    void setTransparentRendering(bool enable);
    void setBackgroundColor(int r, int g, int b, int a = 255);
    QString text(const QRectF &rect) const;

private:
    Page(PagePrivate *pagep)
        : d(pagep)
    {
    }

    PagePrivate *d;

friend class Document;
};

} // end namespace MuPDF

#endif // end MUPDF_PAGE_H
