#include "QMuPDFReader.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QPainter>
#include <QPrinter>

QMuPDFReader::QMuPDFReader(QWidget *parent)
    : QWidget(parent)
	, m_isMouseDown(false)
	, m_lastMouseY(0)
{
    ui.setupUi(this);

	//滑动条样式优化
	QFile QSS1(":/qss/qrc/qss/whiteScrollbar.qss");
	if (QSS1.open(QIODevice::ReadOnly)) {
		QString strStyle = QSS1.readAll();
		ui.scrollArea->verticalScrollBar()->setStyleSheet(strStyle);
	}

	connect(ui.pushButton_close, &QPushButton::clicked, this, &QWidget::close);
	connect(ui.pushButton_openPDF, &QPushButton::clicked, this, &QMuPDFReader::sltOpenPDF);
	connect(ui.pushButton_prevPage, &QPushButton::clicked, this, &QMuPDFReader::sltPreviousPage);
	connect(ui.pushButton_zoomIn, &QPushButton::clicked, this, &QMuPDFReader::sltZoomIn);
	connect(ui.pushButton_zoomOut, &QPushButton::clicked, this, &QMuPDFReader::sltZoomOut);
	connect(ui.pushButton_nextPage, &QPushButton::clicked, this, &QMuPDFReader::sltNextPage);
	connect(ui.pushButton_printer, &QPushButton::clicked, this, &QMuPDFReader::sltPrinterPDF);
	connect(ui.pdfPages, &SequentialPageWidget::updatePdfInfo, this, &QMuPDFReader::sltUpdateInfo);
}

QMuPDFReader::~QMuPDFReader()
{
}

void QMuPDFReader::sltOpenPDF()
{
	QString file = QFileDialog::getOpenFileName(this,
		tr("Open PDF/XPS file"), ".", "PDF (*.pdf);;XPS (*.xps)");
	if (file.isEmpty()){
		QMessageBox::information(NULL, QStringLiteral("提示"), QStringLiteral("请选择正确的PDF文件"));
		return;
	}
	// 打开PDF文档
	bool flag = ui.pdfPages->setDocument(file);
	if (!flag){
		QMessageBox::information(NULL, QStringLiteral("提示"), QStringLiteral("打开PDF文件失败"));
		return;
	}
	ui.label_pdfFileName->setText(file.split("/").last());
}

void QMuPDFReader::sltPreviousPage()
{
	ui.pdfPages->previousPage();
	ui.scrollArea->verticalScrollBar()->setValue(ui.pdfPages->yForPage());
}

void QMuPDFReader::sltNextPage()
{
	ui.pdfPages->nextPage();
	ui.scrollArea->verticalScrollBar()->setValue(ui.pdfPages->yForPage());
}

void QMuPDFReader::sltZoomIn()
{
	ui.pdfPages->zoomIn();
	ui.scrollArea->verticalScrollBar()->setValue(ui.pdfPages->yForPage());
}

void QMuPDFReader::sltZoomOut()
{
	ui.pdfPages->zoomOut();
	ui.scrollArea->verticalScrollBar()->setValue(ui.pdfPages->yForPage());
}

void QMuPDFReader::sltPrinterPDF()
{
	QPrinter printer(QPrinter::HighResolution);
	//printer.setOrientation(QPrinter::Portrait);
	printer.setOutputFormat(QPrinter::NativeFormat);
	printer.setFullPage(true);
	printer.setPageSize(QPageSize(QPageSize::A4));
	//保存成图片
	QImage img;
	QPainter painter(&printer);
	for (int index = 0; index < 1; index++)
	{
		img = ui.pdfPages->getPDFImage(index);
		//painter.drawImage(0,0,img);
		//painter.end();
		QRect rect = painter.viewport();
		QSize size = img.size();
		size.scale(rect.size(), Qt::KeepAspectRatio);
		painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
		painter.setWindow(img.rect());
		painter.drawImage(0, 0, img);
		painter.end();
		img.save("test.bmp");
	}
	QMessageBox::information(NULL, QStringLiteral("提示"), QStringLiteral("打印成功"));
}

void QMuPDFReader::sltGoToPage()
{
	int page = 0;
	QString pagetext = ui.lineEdit_pageInfo->text();
	QString pageinfo = pagetext.split('/').first();
	if (pageinfo.isEmpty()){
		page = ui.lineEdit_pageInfo->text().toInt() - 1;
	}
	else{
		page = pageinfo.toInt() - 1;
	}
	ui.pdfPages->goToPage(page);
	ui.scrollArea->verticalScrollBar()->setValue(ui.pdfPages->yForPage());
}

void QMuPDFReader::sltUpdateInfo(int pageIndex, int totalPages, qreal zoom)
{
	ui.lineEdit_pageInfo->setText(QString("%1/%2").arg(pageIndex + 1).arg(totalPages));
	ui.label_zoomFactor->setText(QString("%1%").arg(qRound(zoom * 100)));
}

void QMuPDFReader::mousePressEvent(QMouseEvent * event)
{
	m_isMouseDown = true;
	m_lastMouseY = event->y();
	QWidget::mousePressEvent(event);
}

void QMuPDFReader::mouseReleaseEvent(QMouseEvent * event)
{
	m_isMouseDown = false;
	QWidget::mouseReleaseEvent(event);
}

void QMuPDFReader::mouseMoveEvent(QMouseEvent * event)
{
	if (m_isMouseDown){
		int value = ui.scrollArea->verticalScrollBar()->value() + m_lastMouseY - event->y();
		m_lastMouseY = event->y();
		ui.scrollArea->verticalScrollBar()->setValue(value);
	}
	QWidget::mouseMoveEvent(event);
}
