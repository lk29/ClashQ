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
}

ConnectionPage::~ConnectionPage()
{
    delete ui;
}
