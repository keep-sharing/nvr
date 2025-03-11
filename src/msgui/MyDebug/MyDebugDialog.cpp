#include "MyDebugDialog.h"
#include "ui_MyDebugDialog.h"
#include "MyDebug.h"
#include "AutoLogoutTip.h"
#include "BasePlayback.h"
#include "MessageBox.h"
#include "MsWaitting.h"
#include "settingcontent.h"
#include "SettingTimeoutTip.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QMouseEvent>
#include <QRegExp>

MyDebugDialog::MyDebugDialog(QWidget *parent)
    : BaseShadowDialog(parent)
{
    QFile file(":/style/style/settingstyle.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(file.readAll());
    } else {
        qWarning() << QString("MyDebugDialog load style failed.");
    }
    file.close();

    ui = new Ui::MyDebugDialog;
    ui->setupUi(this);
    setWindowModality(Qt::NonModal);
    setTitleWidget(ui->label_title);
    setMouseTracking(true);
    ui->widget_background->setMouseTracking(true);

    ui->tabBar->addTab("Debug");
    ui->tabBar->addTab("Setting");
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));
    ui->tabBar->setCurrentTab(0);

    //
    connect(&gDebug, SIGNAL(message(QString)), this, SLOT(onShowDebugInfo(QString)));
}

MyDebugDialog::~MyDebugDialog()
{
    qDebug() << "MyDebugDialog::~MyDebugDialog()";
    delete ui;
}

MyDebugDialog &MyDebugDialog::instance()
{
    static MyDebugDialog self;
    return self;
}

void MyDebugDialog::closeEvent(QCloseEvent *event)
{
    BaseShadowDialog::closeEvent(event);
}

void MyDebugDialog::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    QRect rc = geometry();
    rc.setLeft(rc.right() - 20);
    rc.setTop(rc.bottom() - 20);
    if (rc.contains(event->globalPos())) {
        m_resizeMode = ResizeBottomRight;
        m_tempGeometry = geometry();
        m_pressDistance = m_tempGeometry.bottomRight() - event->globalPos();
    }
}

void MyDebugDialog::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    m_resizeMode = ResizeNone;
    unsetCursor();
}

void MyDebugDialog::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_resizeMode) {
    case ResizeBottomRight:
        m_tempGeometry.setBottomRight(event->globalPos() + m_pressDistance);
        setGeometry(m_tempGeometry);
        break;
    default: {
        QRect rc = geometry();
        rc.setLeft(rc.right() - 20);
        rc.setTop(rc.bottom() - 20);
        if (rc.contains(event->globalPos())) {
            setCursor(Qt::SizeFDiagCursor);
        } else {
            unsetCursor();
        }
        break;
    }
    }
}

void MyDebugDialog::onTabBarClicked(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void MyDebugDialog::onShowDebugInfo(const QString &str)
{
    if (isVisible()) {
        ui->textEdit->append(str);
    }
}

void MyDebugDialog::on_toolButton_close_clicked()
{
    close();
}

void MyDebugDialog::on_pushButton_clear_clicked()
{
    const int result = MessageBox::question(this, "Are you sure to clear?");
    if (result == MessageBox::Yes) {
        ui->textEdit->clear();
    }
}

void MyDebugDialog::on_pushButton_add_clicked()
{
    QString text = ui->comboBox_category->currentText();
    gDebug.setDebugCategory(text);

    //
    ui->listWidget_category->addItem(text);
}

void MyDebugDialog::on_pushButton_delete_clicked()
{
    int row = ui->listWidget_category->currentRow();
    QListWidgetItem *item = ui->listWidget_category->takeItem(row);
    if (item) {
        QString text = item->text();
        gDebug.clearDebugCategory(text);
        delete item;
    }
}
