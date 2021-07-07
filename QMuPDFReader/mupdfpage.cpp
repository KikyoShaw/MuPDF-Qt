#include "mupdfpage.h"
#include "mupdfpage_p.h"
#include "mupdfdocument.h"
#include "mupdfdocument_p.h"
#include "fitz.h"

#include <QImage>
#include <QSizeF>
#include <QDebug>

static void clear_rgb_samples_with_value(
        unsigned char *samples, int size,
        int b, int g, int r, int a)
{
    int i = 0;

    while (i < size)
    {
        *(samples + i++) = r;
        *(samples + i++) = g;
        *(samples + i++) = b;
        *(samples + i++) = a;
    }
}

/**
 * @brief Clean up image data when the last copy of the QImage is destoryed.
 */
static inline void imageCleanupHandler(void *data)
{
    unsigned char *samples = static_cast<unsigned char *>(data);

    if (samples)
    {
        delete []samples;
    }
}

namespace MuPDF
{

Page::~Page()
{
    delete d;
    d = NULL;
}

PagePrivate::PagePrivate(DocumentPrivate *dp, int index)
    : documentp(dp)
    , context(documentp->context)
    , document(documentp->document)
    , page(NULL)
    , display_list(NULL)
    , transparent(documentp->transparent)
    , b(documentp->b), g(documentp->g), r(documentp->r), a(documentp->a)
{
    fz_try(context)
    {
        fz_device *list_device;

        // load page
        page = fz_load_page(context, document, index);

        // display list
        display_list = fz_new_display_list(context, NULL);
        list_device = fz_new_list_device(context, display_list);
        fz_run_page_contents(context, page, list_device, &fz_identity, NULL);
        fz_close_device(context, list_device);
        fz_drop_device(context, list_device);
    }
    fz_catch(context)
    {
        deleteData();
    }
}

/**
 * @brief Check whether this page object is valid.
 */
bool Page::isValid() const
{
    return (d && d->page) ? true : false;
}

/**
 * @brief Render page to QImage
 *
 * @param scaleX scale for X direction
 *               (Default value: 1.0f; >1.0f: zoom in; <1.0f: zoom out)
 * @param scaleY scale for Y direction
 *               (Default value: 1.0f; >1.0f: zoom in; <1.0f: zoom out)
 * @param rotation degree of clockwise rotation (Range: [0.0f, 360.0f))
 *
 * @return This function will return a empty QImage if failed.
 */
QImage Page::renderImage(float scaleX, float scaleY, float rotation) const
{
    fz_pixmap *pixmap = NULL;
    unsigned char *samples = NULL;
    unsigned char *copyed_samples = NULL;
    int width = 0;
    int height = 0;
    int size = 0;

    fz_rect mediabox;
    fz_stext_page * text_page = fz_new_stext_page(d->context, fz_bound_page(d->context, d->page, &mediabox));

    fz_device *tdev;
    tdev = fz_new_stext_device(d->context, text_page, NULL);
    fz_run_display_list(d->context, d->display_list, tdev, &fz_identity, &fz_infinite_rect, NULL);
    fz_close_device(d->context, tdev);
    fz_drop_device(d->context, tdev);

    // build transform matrix
    fz_matrix transform;
    fz_pre_rotate(fz_scale(&transform, scaleX, scaleY), rotation);

    // get transformed page size
    fz_rect bounds;
    fz_irect bbox;
    fz_bound_page(d->context, d->page, &bounds);
    fz_round_rect(&bbox, fz_transform_rect(&bounds, &transform));
    fz_rect_from_irect(&bounds, &bbox);

    // render to pixmap
    fz_device *dev = NULL;
    fz_try(d->context)
    {
        // fz_pixmap will always include a separate alpha channel
        pixmap = fz_new_pixmap_with_bbox(d->context, fz_device_rgb(d->context), &bbox, NULL, 1);

        if (!d->transparent)
        {
            if (d->b >= 0 && d->g >= 0 && d->r >= 0 && d->a >= 0) {
                // with user defined background color
                clear_rgb_samples_with_value(samples, size, d->b, d->g, d->r, d->a);
            }
            else
            {
                // with white background
                fz_clear_pixmap_with_value(d->context, pixmap, 0xff);
            }
        }
        dev = fz_new_draw_device(d->context, NULL, pixmap);
        fz_run_display_list(d->context, d->display_list, dev, &transform, &bounds, NULL);

        samples = fz_pixmap_samples(d->context, pixmap);
        width = fz_pixmap_width(d->context, pixmap);
        height = fz_pixmap_height(d->context, pixmap);
        size = width * height * 4;
    }
    fz_always(d->context)
    {
        if (dev)
        {
            fz_close_device(d->context, dev);
            fz_drop_device(d->context, dev);
        }
        dev = NULL;
    }
    fz_catch(d->context)
    {
        if (pixmap)
        {
            fz_drop_pixmap(d->context, pixmap);
        }
        pixmap = NULL;
    }

    // render to QImage
    QImage image;
    if (NULL == pixmap)
    {
        return image;
    }
    copyed_samples = new unsigned char[size];
    memcpy(copyed_samples, samples, size);
    fz_drop_pixmap(d->context, pixmap);

    image = QImage(copyed_samples,
            width, height, QImage::Format_RGBA8888, imageCleanupHandler, copyed_samples);

    return image;
}

/**
 * @brief %Page size at 72 dpi
 */
QSizeF Page::size() const
{
    fz_rect rect;
    fz_bound_page(d->context, d->page, &rect);
    return QSizeF(rect.x1 - rect.x0, rect.y1 - rect.y0);
}

/**
 * @brief Whether to do transparent page rendering.
 * This function modify setting of current page only.
 * For global setting, use Document::setTransparentRendering() instead.
 *
 * @param enable True: transparent; False: not transparent(default).
 */
void Page::setTransparentRendering(bool enable)
{
    d->transparent = enable;
}

/**
 * @brief Set background color.
 * This function modify setting of current page only.
 * For global setting, use Document::setBackgroundColor() instead.
 *
 * @note This function will only work when page is not transparent.
 * This function won't change content of your file, it only change
 * the way you render the page.
 *
 * @param r red channel
 * @param g green channel
 * @param b blue channel
 * @param a alpha channel(default with non transparent)
 */
void Page::setBackgroundColor(int r, int g, int b, int a)
{
    d->r = r;
    d->g = g;
    d->b = b;
    d->a = a;
}

PagePrivate::~PagePrivate()
{
    if (page) 
    {
        deleteData();
        documentp->pages.removeAt(documentp->pages.indexOf(this));
    }
}

} // end namespace MuPDF
