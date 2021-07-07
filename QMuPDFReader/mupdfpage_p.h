#ifndef MUPDF_PAGE_P_H
#define MUPDF_PAGE_P_H

#include "fitz.h"

namespace MuPDF
{

class DocumentPrivate;

class PagePrivate
{
public:
    PagePrivate(DocumentPrivate *dp, int index);
    ~PagePrivate();

    void deleteData()
    {
        if (display_list)
        {
            fz_drop_display_list(context, display_list);
            display_list = NULL;
        }
        if (page)
        {
            fz_drop_page(context, page);
            page = NULL;
        }
    }

    DocumentPrivate *documentp;
    fz_context *context;
    fz_document *document;
    fz_page *page;
    fz_display_list *display_list;
    bool transparent;
    int b, g, r, a; // background color
};

}

#endif // end MUPDF_PAGE_P_H
