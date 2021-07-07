#pragma once

#include <QtWidgets/QWidget>
#include "ui_QMuPDFReader.h"

class QMuPDFReader : public QWidget
{
    Q_OBJECT

public:
    QMuPDFReader(QWidget *parent = Q_NULLPTR);
	~QMuPDFReader();

protected slots:
	//打开PDF
	void sltOpenPDF();
	//上一页
	void sltPreviousPage();
	//下一页
	void sltNextPage();
	//放大
	void sltZoomIn();
	//缩小
	void sltZoomOut();
	//打印
	void sltPrinterPDF();
	//跳转
	void sltGoToPage();
	//更新
	void sltUpdateInfo(int pageIndex, int totalPages, qreal zoom);

private:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::QMuPDFReaderClass ui;
	bool m_isMouseDown;
	int m_lastMouseY;
};
