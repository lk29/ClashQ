#include "ConnectionModel.h"
#include "ConnectionPage.h"
#include "ConnectionSortModel.h"
#include "ui_ConnectionPage.h"

ConnectionPage::ConnectionPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionPage)
{
    ui->setupUi(this);

    ConnectionSortModel *proxyModel = new ConnectionSortModel(this);
    proxyModel->setSourceModel(new ConnectionModel(this));
    ui->treeView->setModel(proxyModel);
    ui->treeView->sortByColumn(ConnectionModel::HeaderDownload, Qt::DescendingOrder);

    QHeaderView *hdrView = ui->treeView->header();
    hdrView->resizeSection(ConnectionModel::HeaderUpload, 120);
    hdrView->resizeSection(ConnectionModel::HeaderDownload, 120);
    hdrView->resizeSection(ConnectionModel::HeaderChains, 180);
    hdrView->resizeSection(ConnectionModel::HeaderTime, 130);
    hdrView->resizeSection(ConnectionModel::HeaderSource, 150);
    hdrView->resizeSection(ConnectionModel::HeaderDestIp, 150);
    hdrView->resizeSection(ConnectionModel::HeaderType, 150);

    hdrView->setSectionResizeMode(QHeaderView::Stretch);
    hdrView->setSectionResizeMode(ConnectionModel::HeaderUpload, QHeaderView::Fixed);
    hdrView->setSectionResizeMode(ConnectionModel::HeaderDownload, QHeaderView::Fixed);
    hdrView->setSectionResizeMode(ConnectionModel::HeaderChains, QHeaderView::Fixed);
    hdrView->setSectionResizeMode(ConnectionModel::HeaderTime, QHeaderView::Fixed);
    hdrView->setSectionResizeMode(ConnectionModel::HeaderSource, QHeaderView::Fixed);
    hdrView->setSectionResizeMode(ConnectionModel::HeaderDestIp, QHeaderView::Fixed);
    hdrView->setSectionResizeMode(ConnectionModel::HeaderType, QHeaderView::Fixed);
}

ConnectionPage::~ConnectionPage()
{
    delete ui;
}
