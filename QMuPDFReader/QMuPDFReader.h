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
	//��PDF
	void sltOpenPDF();
	//��һҳ
	void sltPreviousPage();
	//��һҳ
	void sltNextPage();
	//�Ŵ�
	void sltZoomIn();
	//��С
	void sltZoomOut();
	//��ӡ
	void sltPrinterPDF();
	//��ת
	void sltGoToPage();
	//����
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
