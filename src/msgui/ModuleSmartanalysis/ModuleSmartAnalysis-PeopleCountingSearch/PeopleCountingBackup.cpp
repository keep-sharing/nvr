#include "PeopleCountingBackup.h"
#include "CameraPeopleCountingHistogramScene.h"
#include "CameraPeopleCountingPolylineScene.h"
#include "HistogramScene.h"
#include "LineScene.h"
#include "MsDevice.h"
#include "MyDebug.h"
#include "RegionPeopleCountingHistogramScene.h"
#include "RegionPeopleCountingPolylineScene.h"
#include "myqt.h"
#include <QDateTime>
#include <QPainter>
#include <QPrinter>

PeopleCountingBackup::PeopleCountingBackup(QObject *parent)
    : QThread(parent)
{
}

void PeopleCountingBackup::startBackup(const ExportPeopleCountingInfo &info)
{
    m_info = info;

    m_recordType = gPeopleCountingData.reportType();
    m_statisticsType = gPeopleCountingData.statisticsType();
    m_startDateTime = gPeopleCountingData.startDateTime();

    m_groupNameMap = gPeopleCountingData.groupsName();
    m_groupChannelsMap = gPeopleCountingData.groupsChannels();
    m_groupAllChannelsMap = gPeopleCountingData.groupsAllChannels();

    start();
}

void PeopleCountingBackup::setChartMode(const ChartMode &mode)
{
    m_chartMode = mode;
}

void PeopleCountingBackup::setMainTitle(const QString &text)
{
    m_mainTitle = text;
}

void PeopleCountingBackup::setSubTitle(const QString &text)
{
    m_subTitle = text;
}

void PeopleCountingBackup::setSubTitleForBackup(const QString &text)
{
    m_subTitleForBackup = text;
}

void PeopleCountingBackup::setGroups(const QList<int> &groups)
{
    m_groups = groups;
}

void PeopleCountingBackup::setSelectTypes(const QList<int> &typeList)
{
    m_selectTypes = typeList;
}

void PeopleCountingBackup::setViewportGeometry(const QRect &rc)
{
    m_viewportGeometry = rc;
}

void PeopleCountingBackup::run()
{
    int result = 0;
    switch (m_info.fileFormat) {
    case PeopleCounting_CSV: {
        switch (gPeopleCountingData.searchType()) {
        case PEOPLECNT_SEARCH_BY_CAMERA:
            result = backupCameraCSV();
            break;
        case PEOPLECNT_SEARCH_BY_GROUP:
            result = backupGroupCSV();
            break;
        case PEOPLECNT_SEARCH_BY_REGION:
            result = backupRegionCSV();
            break;
        default:
            break;
        }
        break;
    }
    case PeopleCounting_PDF: {
        switch (gPeopleCountingData.searchType()) {
        case PEOPLECNT_SEARCH_BY_CAMERA:
            result = backupCameraPDF();
            break;
        case PEOPLECNT_SEARCH_BY_GROUP:
            result = backupGroupPDF();
            break;
        case PEOPLECNT_SEARCH_BY_REGION:
            result = backupRegionPDF();
            break;
        default:
            break;
        }
        break;
    }
    case PeopleCounting_PNG: {
        switch (gPeopleCountingData.searchType()) {
        case PEOPLECNT_SEARCH_BY_CAMERA:
            result = backupCameraPNG();
            break;
        case PEOPLECNT_SEARCH_BY_GROUP:
            result = backupGroupPNG();
            break;
        case PEOPLECNT_SEARCH_BY_REGION:
            result = backupRegionPNG();
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
    sync();

    ms_system("echo 3 > /proc/sys/vm/drop_caches");

    emit backupFinished(result);
}

int PeopleCountingBackup::backupCameraCSV()
{
    int result = 0;
    for (auto channel : m_channels) {
        for (int line = 0; line < 5; line++) {
            if (m_lineMask & (1 << line)) {
                QString filePath;
                if (line > 0) {
                    filePath = QString("%1/NVR_People_Counting_by_Camera_CH%2_Line%3_%4.csv")
                                   .arg(m_info.path)
                                   .arg(channel + 1, 2, 10, QLatin1Char('0'))
                                   .arg(line)
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
                } else {
                    filePath = QString("%1/NVR_People_Counting_by_Camera_CH%2_Total_%3.csv")
                                   .arg(m_info.path)
                                   .arg(channel + 1, 2, 10, QLatin1Char('0'))
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
                }
                QFile file(filePath);
                if (!file.open(QFile::WriteOnly)) {
                    result |= BackupResultFailed;
                    continue;
                }

                //
                QTextStream stream(&file);
                stream << "Time"
                       << ",";
                for (int i = 0; i < m_selectTypes.size(); ++i) {
                    int channel = m_selectTypes.at(i);
                    switch (channel) {
                    case -1:
                        stream << "Sum"
                               << ",";
                        break;
                    case 0:
                        stream << "People Entered"
                               << ",";
                        break;
                    case 1:
                        stream << "People Exited"
                               << ",";
                        break;
                    }
                }
                stream << "\n";

                //
                gPeopleCountingData.setCurrentChannel(channel);
                gPeopleCountingData.dealData(m_textMap[channel], line);
                const PeopleGridData &gridData = gPeopleCountingData.peopleLineGridData(channel);
                const PeopleLineData &lineData = gPeopleCountingData.peopleLineData(channel);
                int startX = gridData.xMinValue;
                for (int j = 0; j < gridData.xLineCount; ++j) {
                    QString textDateTime;
                    switch (m_recordType) {
                    case DailyReport: {
                        textDateTime = QString("%1 %2:00-%3:00")
                                           .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                           .arg(startX, 2, 10, QLatin1Char('0'))
                                           .arg(startX + 1, 2, 10, QLatin1Char('0'));
                        break;
                    }
                    case WeeklyReport: {
                        QDateTime dateTime = m_startDateTime.toLocalTime().addDays(startX);
                        textDateTime = QString("%1 %2")
                                           .arg(dateTime.toString("yyyy/MM/dd"))
                                           .arg(MyQt::weekString(dateTime.date().dayOfWeek()));
                        break;
                    }
                    case MonthlyReport: {
                        textDateTime = QString("%1")
                                           .arg(m_startDateTime.toLocalTime().addDays(startX).toString("yyyy/MM/dd"));
                        break;
                    }
                    case YearlyReport: {
                        QDateTime dateTime = m_startDateTime.toLocalTime().addMonths(startX);
                        if (j > 0) {
                            QDate date = dateTime.date();
                            dateTime.setDate(QDate(date.year(), date.month(), 1));
                        }
                        textDateTime = QString("%1").arg(dateTime.toString("yyyy/MM/dd"));
                        break;
                    }
                    default:
                        break;
                    }
                    stream << textDateTime << ",";
                    //
                    for (int i = 0; i < lineData.lines.size(); ++i) {
                        int lineIndex = lineData.lines.at(i);
                        const auto &valueMap = lineData.lineData.value(lineIndex);

                        int yValue = valueMap.value(j, 0);
                        stream << QString("%1").arg(yValue) << ",";
                    }
                    //
                    stream << "\n";
                    startX++;
                }
                result |= BackupResultSuccessed;
            }
        }
    }
    return result;
}

int PeopleCountingBackup::backupGroupCSV()
{
    int result = 0;
    for (int i = 0; i < m_groups.size(); ++i) {
        int group = m_groups.at(i);
        //const QString &groupName = m_groupNameMap.value(group);
        const auto &channels = m_groupAllChannelsMap.value(group);

        QString filePath = QString("%1/NVR_People_Counting_by_Group_Group%2_%3.csv")
                               .arg(m_info.path)
                               .arg(group + 1, 2, 10, QLatin1Char('0'))
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        int itemResult = backupCSV(filePath, group, channels);
        if (itemResult == 0) {
            result |= BackupResultSuccessed;
        } else {
            result |= BackupResultFailed;
            qMsWarning() << "save csv error:" << filePath;
        }
    }
    return result;
}

int PeopleCountingBackup::backupRegionCSV()
{
    int result = 0;
    for (auto channel : m_channels) {
        QString filePath = QString("%1/NVR_Regional_People_Counting_CH%2_%3.csv")
                               .arg(m_info.path)
                               .arg(channel + 1, 2, 10, QLatin1Char('0'))
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        QFile file(filePath);
        if (!file.open(QFile::WriteOnly)) {
            result |= BackupResultFailed;
            continue;
        }

        //
        QTextStream stream(&file);
        stream << "Time"
               << ",";
        stream << "Total"
               << ",";
        for (int i = 0; i < m_selectTypes.size(); ++i) {
            int channel = m_selectTypes.at(i);
            switch (channel) {
            case 0:
                stream << "Region1"
                       << ",";
                break;
            case 1:
                stream << "Region2"
                       << ",";
                break;
            case 2:
                stream << "Region3"
                       << ",";
                break;
            case 3:
                stream << "Region4"
                       << ",";
                break;
            }
        }
        stream << "\r\n";

        //
        const PeopleGridData &gridData = gPeopleCountingData.peopleLineGridData(channel);
        const PeopleLineData &lineData = gPeopleCountingData.peopleLineData(channel);
        int startX = gridData.xMinValue;
        for (int j = 0; j < gridData.xLineCount; ++j) {
            QString textDateTime;
            switch (m_recordType) {
            case DailyReport: {
                textDateTime = QString("%1 %2:00-%3:00")
                                   .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                   .arg(startX, 2, 10, QLatin1Char('0'))
                                   .arg(startX + 1, 2, 10, QLatin1Char('0'));
                break;
            }
            case WeeklyReport: {
                QDateTime dateTime = m_startDateTime.toLocalTime().addDays(startX);
                textDateTime = QString("%1 %2")
                                   .arg(dateTime.toString("yyyy/MM/dd"))
                                   .arg(MyQt::weekString(dateTime.date().dayOfWeek()));
                break;
            }
            case MonthlyReport: {
                textDateTime = QString("%1")
                                   .arg(m_startDateTime.toLocalTime().addDays(startX).toString("yyyy/MM/dd"));
                break;
            }
            default:
                break;
            }
            stream << textDateTime << ",";
            //
            for (int i = 0; i < lineData.lines.size(); ++i) {
                int lineIndex = lineData.lines.at(i);
                const auto &valueMap = lineData.lineData.value(lineIndex);

                int yValue = valueMap.value(j, 0);
                stream << QString("%1").arg(yValue) << ",";
            }
            //
            stream << "\r\n";
            startX++;
        }
        result |= BackupResultSuccessed;
    }

    return result;
}

int PeopleCountingBackup::backupCameraPDF()
{
    int result = 0;
    for (auto channel : m_channels) {
        for (int line = 0; line < 5; line++) {
            if (m_lineMask & (1 << line)) {
                QString filePath;
                QString subTitle;
                if (line > 0) {
                    filePath = QString("%1/NVR_People_Counting_by_Camera_CH%2_Line%3_%4.pdf")
                                   .arg(m_info.path)
                                   .arg(channel + 1, 2, 10, QLatin1Char('0'))
                                   .arg(line)
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
                    subTitle = (QString("%1 - %2 - %3 - %4 - Line%5")
                                    .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.statisticsTypeString())
                                    .arg(qMsNvr->channelName(channel))
                                    .arg(line));
                } else {
                    filePath = QString("%1/NVR_People_Counting_by_Camera_CH%2_Total_%3.pdf")
                                   .arg(m_info.path)
                                   .arg(channel + 1, 2, 10, QLatin1Char('0'))
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
                    subTitle = (QString("%1 - %2 - %3 - %4 - Total")
                                    .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.statisticsTypeString())
                                    .arg(qMsNvr->channelName(channel)));
                }
                QFile file(filePath);
                if (!file.open(QFile::WriteOnly)) {
                    result |= BackupResultFailed;
                    continue;
                }

                //
                QPrinter printer;
                printer.setPageSize(QPrinter::A4);
                printer.setOrientation(QPrinter::Landscape);
                printer.setOutputFileName(filePath);
                printer.setOutputFormat(QPrinter::PdfFormat);

                QPainter pdfPainter(&printer);
                pdfPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

                //
                QRect imageRc(QPoint(0, 0), m_viewportGeometry.size());
                QRect pdfRc = pdfPainter.viewport();
                qreal r1 = (qreal)imageRc.width() / imageRc.height();
                qreal r2 = (qreal)pdfRc.width() / pdfRc.height();
                if (r1 > r2) {
                    imageRc.setWidth(pdfRc.width());
                    imageRc.setHeight(pdfRc.width() / r1);
                } else {
                    imageRc.setWidth(pdfRc.height() * r1);
                    imageRc.setHeight(pdfRc.height());
                }
                imageRc.moveCenter(pdfRc.center());

                //
                {
                    gPeopleCountingData.setCurrentChannel(channel);
                    gPeopleCountingData.dealData(m_textMap[channel], line);

                    QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
                    image.fill(Qt::white);

                    QPainter painter(&image);
                    painter.setRenderHints(QPainter::Antialiasing);

                    switch (m_chartMode) {
                    case LineChart: {
                        CameraPeopleCountingPolylineScene *scene = new CameraPeopleCountingPolylineScene;
                        scene->setSceneRect(m_viewportGeometry);
                        scene->setMainTitle(m_mainTitle);
                        scene->setSubTitle(subTitle);
                        scene->showLine(m_selectTypes, channel);
                        scene->render(&painter);
                        scene->deleteLater();
                        break;
                    }
                    case HistogramChart: {
                        CameraPeopleCountingHistogramScene *scene = new CameraPeopleCountingHistogramScene;
                        scene->setSceneRect(m_viewportGeometry);
                        scene->setMainTitle(m_mainTitle);
                        scene->setSubTitle(subTitle);
                        scene->showHistogram(m_selectTypes, channel);
                        scene->render(&painter);
                        scene->deleteLater();
                        break;
                    }
                    case Histogram2Chart: {
                        CameraPeopleCountingHistogramScene *scene = new CameraPeopleCountingHistogramScene;
                        scene->setSceneRect(m_viewportGeometry);
                        scene->setMainTitle(m_mainTitle);
                        scene->setSubTitle(subTitle);
                        scene->showHistogram2(m_selectTypes, channel);
                        scene->render(&painter);
                        scene->deleteLater();
                        break;
                    }
                    default:
                        break;
                    }

                    //
                    pdfPainter.drawImage(imageRc, image);
                }

                //
                printer.newPage();

                //draw csv image
                {
                    QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
                    image.fill(Qt::white);

                    QPainter painter(&image);

                    //
                    const PeopleGridData &gridData = gPeopleCountingData.peopleLineGridData(channel);
                    const PeopleLineData &lineData = gPeopleCountingData.peopleLineData(channel);

                    QRectF gridRc(QPoint(1, 1), painter.viewport().size());
                    gridRc.setRight(gridRc.right() - 2);
                    gridRc.setBottom(gridRc.bottom() - 2);

                    //第一列占2 * perWidth
                    int columnCount = 13;
                    qreal perWidth = gridRc.width() / columnCount;

                    int maxNameRowCount = 2;

                    int rowCount = 31 + maxNameRowCount;
                    qreal perHeight = gridRc.height() / rowCount;

                    //网格
                    {
                        //横线
                        qreal x1 = gridRc.left();
                        qreal y1 = gridRc.top();
                        qreal x2 = gridRc.right();
                        qreal y2 = y1;
                        for (int row = 0; row < gridData.xLineCount + 2; ++row) {
                            painter.drawLine(x1, y1, x2, y2);
                            if (row == 0) {
                                y1 += perHeight * maxNameRowCount;
                                y2 = y1;
                            } else {
                                y1 += perHeight;
                                y2 = y1;
                            }
                        }
                        //竖线
                        x1 = gridRc.left();
                        y1 = gridRc.top();
                        x2 = x1;
                        y2 = gridRc.top() + (gridData.xLineCount + maxNameRowCount) * perHeight;
                        for (int column = 0; column < columnCount; ++column) {
                            painter.drawLine(x1, y1, x2, y2);
                            if (column == 0) {
                                x1 += perWidth * 2;
                                x2 = x1;
                            } else {
                                x1 += perWidth;
                                x2 = x1;
                            }
                        }
                    }

                    //horizontal header
                    {
                        qreal left = gridRc.left();
                        qreal top = gridRc.top();
                        painter.drawText(QRectF(left, top, perWidth * 2, perHeight * maxNameRowCount), Qt::AlignCenter, "Time");
                        left += perWidth * 2;
                        //
                        for (int i = 0; i < m_selectTypes.size(); ++i) {
                            int channel = m_selectTypes.at(i);
                            QString text;
                            switch (channel) {
                            case -1:
                                text = "Sum";
                                break;
                            case 0:
                                text = "People Entered";
                                break;
                            case 1:
                                text = "People Exited";
                                break;
                            }
                            painter.drawText(QRectF(left, top, perWidth, perHeight * maxNameRowCount), Qt::AlignCenter, text);
                            left += perWidth;
                        }
                    }

                    //vertical header
                    {
                        qreal left = gridRc.left();
                        qreal top = gridRc.top();
                        top += perHeight * maxNameRowCount;
                        //
                        int startX = gridData.xMinValue;
                        for (int i = 0; i < gridData.xLineCount; ++i) {
                            QString textDateTime;
                            switch (m_recordType) {
                            case DailyReport:
                                textDateTime = QString("%1 %2:00-%3:00")
                                                   .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                                   .arg(startX, 2, 10, QLatin1Char('0'))
                                                   .arg(startX + 1, 2, 10, QLatin1Char('0'));
                                break;
                            case WeeklyReport:
                                textDateTime = QString("%1 %2")
                                                   .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                                   .arg(MyQt::weekString(startX + 1));
                                break;
                            case MonthlyReport:
                                textDateTime = QString("%1")
                                                   .arg(m_startDateTime.toLocalTime().addDays(startX).toString("yyyy/MM/dd"));
                                break;
                            case YearlyReport: {
                                QDateTime dateTime = m_startDateTime.toLocalTime().addMonths(startX);
                                if (i > 0) {
                                    QDate date = dateTime.date();
                                    dateTime.setDate(QDate(date.year(), date.month(), 0));
                                }
                                textDateTime = QString("%1").arg(dateTime.toString("yyyy/MM/dd"));
                                break;
                            }
                            default:
                                break;
                            }
                            QTextOption textOption(Qt::AlignCenter);
                            textOption.setWrapMode(QTextOption::WrapAnywhere);
                            painter.drawText(QRectF(left, top, perWidth * 2, perHeight), textDateTime, textOption);
                            //
                            top += perHeight;
                            startX++;
                        }
                    }

                    //value
                    qreal left = gridRc.left();
                    left += perWidth * 2;
                    qreal top = gridRc.top();
                    top += perHeight * maxNameRowCount;
                    for (int j = 0; j < gridData.xLineCount; ++j) {
                        for (int i = 0; i < lineData.lines.size(); ++i) {
                            int lineIndex = lineData.lines.at(i);
                            const auto &valueMap = lineData.lineData.value(lineIndex);
                            int yValue = valueMap.value(j, 0);
                            painter.drawText(QRectF(left, top, perWidth, perHeight), Qt::AlignCenter, QString("%1").arg(yValue));
                            left += perWidth;
                        }
                        //
                        left = gridRc.left() + perWidth * 2;
                        top += perHeight;
                    }

                    //
                    pdfPainter.drawImage(imageRc, image);
                    result |= BackupResultSuccessed;
                }
            }
        }
    }

    return result;
}

int PeopleCountingBackup::backupGroupPDF()
{
    int result = 0;
    for (int i = 0; i < m_groups.size(); ++i) {
        int group = m_groups.at(i);
        //const QString &groupName = m_groupNameMap.value(group);
        auto channels = m_groupChannelsMap.value(group);
        channels.prepend(TOTAL_LINE_INDEX);

        QString filePath = QString("%1/NVR_People_Counting_by_Group_Group%2_%3.pdf")
                               .arg(m_info.path)
                               .arg(group + 1, 2, 10, QLatin1Char('0'))
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        int itemResult = backupPDF(filePath, group, channels);
        if (itemResult == 0) {
            result |= BackupResultSuccessed;
        } else {
            result |= BackupResultFailed;
            qMsWarning() << "save pdf error:" << filePath;
        }
    }
    return result;
}

int PeopleCountingBackup::backupRegionPDF()
{
    int result = 0;
    for (auto channel : m_channels) {
        QString filePath = QString("%1/NVR_Regional_People_Counting_CH%2_%3.pdf")
                               .arg(m_info.path)
                               .arg(channel + 1, 2, 10, QLatin1Char('0'))
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        QFile file(filePath);
        if (!file.open(QFile::WriteOnly)) {
            result |= BackupResultFailed;
            continue;
        }

        //
        QPrinter printer;
        printer.setPageSize(QPrinter::A4);
        printer.setOrientation(QPrinter::Landscape);
        printer.setOutputFileName(filePath);
        printer.setOutputFormat(QPrinter::PdfFormat);

        QPainter pdfPainter(&printer);
        pdfPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        //
        QRect imageRc(QPoint(0, 0), m_viewportGeometry.size());
        QRect pdfRc = pdfPainter.viewport();
        qreal r1 = (qreal)imageRc.width() / imageRc.height();
        qreal r2 = (qreal)pdfRc.width() / pdfRc.height();
        if (r1 > r2) {
            imageRc.setWidth(pdfRc.width());
            imageRc.setHeight(pdfRc.width() / r1);
        } else {
            imageRc.setWidth(pdfRc.height() * r1);
            imageRc.setHeight(pdfRc.height());
        }
        imageRc.moveCenter(pdfRc.center());

        //
        {
            QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
            image.fill(Qt::white);

            QPainter painter(&image);
            painter.setRenderHints(QPainter::Antialiasing);

            QString subTitle = QString("%1 - %2 - %3")
                                   .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                   .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                   .arg(qMsNvr->channelName(channel));

            switch (m_chartMode) {
            case LineChart: {
                RegionPeopleCountingPolylineScene *scene = new RegionPeopleCountingPolylineScene;
                scene->setSceneRect(m_viewportGeometry);
                scene->setMainTitle(m_mainTitle);
                scene->setSubTitle(subTitle);
                scene->showLine(m_selectTypes, channel);
                scene->render(&painter);
                scene->deleteLater();
                break;
            }
            case HistogramChart: {
                RegionPeopleCountingHistogramScene *scene = new RegionPeopleCountingHistogramScene;
                scene->setSceneRect(m_viewportGeometry);
                scene->setMainTitle(m_mainTitle);
                scene->setSubTitle(subTitle);
                scene->showHistogram(m_selectTypes, channel);
                scene->render(&painter);
                scene->deleteLater();
                break;
            }
            case Histogram2Chart: {
                RegionPeopleCountingHistogramScene *scene = new RegionPeopleCountingHistogramScene;
                scene->setSceneRect(m_viewportGeometry);
                scene->setMainTitle(m_mainTitle);
                scene->setSubTitle(subTitle);
                scene->showHistogram2(m_selectTypes, channel);
                scene->render(&painter);
                scene->deleteLater();
                break;
            }
            default:
                break;
            }

            //
            pdfPainter.drawImage(imageRc, image);
        }

        //
        printer.newPage();

        //draw csv image
        {
            QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
            image.fill(Qt::white);

            QPainter painter(&image);

            const PeopleGridData &gridData = gPeopleCountingData.peopleLineGridData(channel);
            const PeopleLineData &lineData = gPeopleCountingData.peopleLineData(channel);

            QRectF gridRc(QPoint(1, 1), painter.viewport().size());
            gridRc.setRight(gridRc.right() - 2);
            gridRc.setBottom(gridRc.bottom() - 2);

            //第一列占2 * perWidth
            int columnCount = 13;
            qreal perWidth = gridRc.width() / columnCount;

            int maxNameRowCount = 2;

            int rowCount = 31 + maxNameRowCount;
            qreal perHeight = gridRc.height() / rowCount;

            //网格
            {
                //横线
                qreal x1 = gridRc.left();
                qreal y1 = gridRc.top();
                qreal x2 = gridRc.right();
                qreal y2 = y1;
                for (int row = 0; row < gridData.xLineCount + 2; ++row) {
                    painter.drawLine(x1, y1, x2, y2);
                    if (row == 0) {
                        y1 += perHeight * maxNameRowCount;
                        y2 = y1;
                    } else {
                        y1 += perHeight;
                        y2 = y1;
                    }
                }
                //竖线
                x1 = gridRc.left();
                y1 = gridRc.top();
                x2 = x1;
                y2 = gridRc.top() + (gridData.xLineCount + maxNameRowCount) * perHeight;
                for (int column = 0; column < columnCount; ++column) {
                    painter.drawLine(x1, y1, x2, y2);
                    if (column == 0) {
                        x1 += perWidth * 2;
                        x2 = x1;
                    } else {
                        x1 += perWidth;
                        x2 = x1;
                    }
                }
            }

            //horizontal header
            {
                qreal left = gridRc.left();
                qreal top = gridRc.top();
                painter.drawText(QRectF(left, top, perWidth * 2, perHeight * maxNameRowCount), Qt::AlignCenter, "Time");
                left += perWidth * 2;
                //
                for (int i = 0; i < m_selectTypes.size(); ++i) {
                    int channel = m_selectTypes.at(i);
                    QString text;
                    switch (channel) {
                    case -1:
                        text = "Total";
                        break;
                    case 0:
                        text = "Region1";
                        break;
                    case 1:
                        text = "Region2";
                        break;
                    case 2:
                        text = "Region3";
                        break;
                    case 3:
                        text = "Region4";
                        break;
                    }
                    painter.drawText(QRectF(left, top, perWidth, perHeight * maxNameRowCount), Qt::AlignCenter, text);
                    left += perWidth;
                }
            }

            //vertical header
            {
                qreal left = gridRc.left();
                qreal top = gridRc.top();
                top += perHeight * maxNameRowCount;
                //
                int startX = gridData.xMinValue;
                for (int i = 0; i < gridData.xLineCount; ++i) {
                    QString textDateTime;
                    switch (m_recordType) {
                    case DailyReport:
                        textDateTime = QString("%1 %2:00-%3:00")
                                           .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                           .arg(startX, 2, 10, QLatin1Char('0'))
                                           .arg(startX + 1, 2, 10, QLatin1Char('0'));
                        break;
                    case WeeklyReport:
                        textDateTime = QString("%1 %2")
                                           .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                           .arg(MyQt::weekString(startX + 1));
                        break;
                    case MonthlyReport:
                        textDateTime = QString("%1")
                                           .arg(m_startDateTime.toLocalTime().addDays(startX).toString("yyyy/MM/dd"));
                        break;
                    default:
                        break;
                    }
                    QTextOption textOption(Qt::AlignCenter);
                    textOption.setWrapMode(QTextOption::WrapAnywhere);
                    painter.drawText(QRectF(left, top, perWidth * 2, perHeight), textDateTime, textOption);
                    //
                    top += perHeight;
                    startX++;
                }
            }

            //value
            qreal left = gridRc.left();
            left += perWidth * 2;
            qreal top = gridRc.top();
            top += perHeight * maxNameRowCount;
            for (int j = 0; j < gridData.xLineCount; ++j) {
                for (int i = 0; i < lineData.lines.size(); ++i) {
                    int lineIndex = lineData.lines.at(i);
                    const auto &valueMap = lineData.lineData.value(lineIndex);
                    int yValue = valueMap.value(j, 0);
                    painter.drawText(QRectF(left, top, perWidth, perHeight), Qt::AlignCenter, QString("%1").arg(yValue));
                    left += perWidth;
                }
                //
                left = gridRc.left() + perWidth * 2;
                top += perHeight;
            }

            //
            pdfPainter.drawImage(imageRc, image);
            result |= BackupResultSuccessed;
        }
    }

    return result;
}

int PeopleCountingBackup::backupCameraPNG()
{
    int result = 0;
    for (auto channel : m_channels) {
        for (int line = 0; line < 5; line++) {
            if (m_lineMask & (1 << line)) {
                QString filePath;
                QString subTitle;
                if (line > 0) {
                    filePath = QString("%1/NVR_People_Counting_by_Camera_CH%2_Line%3_%4.png")
                                   .arg(m_info.path)
                                   .arg(channel + 1, 2, 10, QLatin1Char('0'))
                                   .arg(line)
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
                    subTitle = (QString("%1 - %2 - %3 - %4 - Line%5")
                                    .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.statisticsTypeString())
                                    .arg(qMsNvr->channelName(channel))
                                    .arg(line));
                } else {
                    filePath = QString("%1/NVR_People_Counting_by_Camera_CH%2_Total_%3.png")
                                   .arg(m_info.path)
                                   .arg(channel + 1, 2, 10, QLatin1Char('0'))
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
                    subTitle = (QString("%1 - %2 - %3 - %4 - Total")
                                    .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                                    .arg(gPeopleCountingData.statisticsTypeString())
                                    .arg(qMsNvr->channelName(channel)));
                }

                QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
                image.fill(Qt::white);

                QPainter painter(&image);
                painter.setRenderHints(QPainter::Antialiasing);

                gPeopleCountingData.setCurrentChannel(channel);
                gPeopleCountingData.dealData(m_textMap[channel], line);

                switch (m_chartMode) {
                case LineChart: {
                    CameraPeopleCountingPolylineScene *scene = new CameraPeopleCountingPolylineScene;
                    scene->setSceneRect(m_viewportGeometry);
                    scene->setMainTitle(m_mainTitle);
                    scene->setSubTitle(subTitle);
                    scene->showLine(m_selectTypes, channel);
                    scene->render(&painter);
                    scene->deleteLater();
                    break;
                }
                case HistogramChart: {
                    CameraPeopleCountingHistogramScene *scene = new CameraPeopleCountingHistogramScene;
                    scene->setSceneRect(m_viewportGeometry);
                    scene->setMainTitle(m_mainTitle);
                    scene->setSubTitle(subTitle);
                    scene->showHistogram(m_selectTypes, channel);
                    scene->render(&painter);
                    scene->deleteLater();
                    break;
                }
                case Histogram2Chart: {
                    CameraPeopleCountingHistogramScene *scene = new CameraPeopleCountingHistogramScene;
                    scene->setSceneRect(m_viewportGeometry);
                    scene->setMainTitle(m_mainTitle);
                    scene->setSubTitle(subTitle);
                    scene->showHistogram2(m_selectTypes, channel);
                    scene->render(&painter);
                    scene->deleteLater();
                    break;
                }
                default:
                    break;
                }

                bool ok = image.save(filePath);
                if (!ok) {
                    qMsWarning() << "save image error:" << filePath;
                    result |= BackupResultFailed;
                } else {
                    result |= BackupResultSuccessed;
                }
            }
        }
    }

    return result;
}

int PeopleCountingBackup::backupGroupPNG()
{
    int result = 0;
    for (int i = 0; i < m_groups.size(); ++i) {
        int group = m_groups.at(i);
        //const QString &groupName = m_groupNameMap.value(group);
        auto channels = m_groupChannelsMap.value(group);
        channels.prepend(TOTAL_LINE_INDEX);

        QString filePath = QString("%1/NVR_People_Counting_by_Group_Group%2_%3.png")
                               .arg(m_info.path)
                               .arg(group + 1, 2, 10, QLatin1Char('0'))
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        int itemResult = backupPNG(filePath, group, channels);
        if (itemResult == 0) {
            result |= BackupResultSuccessed;
        } else {
            result |= BackupResultFailed;
            qMsWarning() << "save png error:" << filePath;
        }
    }
    return result;
}

int PeopleCountingBackup::backupRegionPNG()
{
    int result = 0;
    for (auto channel : m_channels) {
        QString filePath = QString("%1/NVR_Regional_People_Counting_CH%2_%3.png")
                               .arg(m_info.path)
                               .arg(channel + 1, 2, 10, QLatin1Char('0'))
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        //

        QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
        image.fill(Qt::white);

        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing);

        QString subTitle = QString("%1 - %2 - %3")
                               .arg(gPeopleCountingData.startDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                               .arg(gPeopleCountingData.endDateTime().toString("yyyy/MM/dd HH:mm:ss"))
                               .arg(qMsNvr->channelName(channel));

        switch (m_chartMode) {
        case LineChart: {
            RegionPeopleCountingPolylineScene *scene = new RegionPeopleCountingPolylineScene;
            scene->setSceneRect(m_viewportGeometry);
            scene->setMainTitle(m_mainTitle);
            scene->setSubTitle(subTitle);
            scene->showLine(m_selectTypes, channel);
            scene->render(&painter);
            scene->deleteLater();
            break;
        }
        case HistogramChart: {
            RegionPeopleCountingHistogramScene *scene = new RegionPeopleCountingHistogramScene;
            scene->setSceneRect(m_viewportGeometry);
            scene->setMainTitle(m_mainTitle);
            scene->setSubTitle(subTitle);
            scene->showHistogram(m_selectTypes, channel);
            scene->render(&painter);
            scene->deleteLater();
            break;
        }
        case Histogram2Chart: {
            RegionPeopleCountingHistogramScene *scene = new RegionPeopleCountingHistogramScene;
            scene->setSceneRect(m_viewportGeometry);
            scene->setMainTitle(m_mainTitle);
            scene->setSubTitle(subTitle);
            scene->showHistogram2(m_selectTypes, channel);
            scene->render(&painter);
            scene->deleteLater();
            break;
        }
        default:
            break;
        }

        bool ok = image.save(filePath);
        if (!ok) {
            qMsWarning() << "save image error:" << filePath;
            result |= BackupResultFailed;
        } else {
            result |= BackupResultSuccessed;
        }
    }

    return result;
}

int PeopleCountingBackup::backupCSV(const QString &filePath, int groupFilter, const QList<int> &channels)
{
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly)) {
        return -1;
    }

    QTextStream stream(&file);
    stream << "Time"
           << ",";

    switch (gPeopleCountingData.statisticsType()) {
    case PeopleEntered:
        stream << "Total Entered"
               << ",";
        break;
    case PeopleExited:
        stream << "Total Exited"
               << ",";
        break;
    case PeopleAll:
        stream << "Total Sum"
               << ",";
        break;
    default:
        break;
    }

    for (int i = 0; i < channels.size(); ++i) {
        int channel = channels.at(i);
        stream << QString("CH%1").arg(channel + 1, 2, 10, QLatin1Char('0')) << ",";
    }

    stream << "\r\n";

    const PeopleGridData &gridData = gPeopleCountingData.peopleLineGridData(groupFilter);
    const PeopleLineData &lineData = gPeopleCountingData.peopleLineData(groupFilter);
    int startX = gridData.xMinValue;
    for (int j = 0; j < gridData.xLineCount; ++j) {
        QString textDateTime;
        switch (m_recordType) {
        case DailyReport: {
            textDateTime = QString("%1 %2:00-%3:00")
                               .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                               .arg(startX, 2, 10, QLatin1Char('0'))
                               .arg(startX + 1, 2, 10, QLatin1Char('0'));
            break;
        }
        case WeeklyReport: {
            QDateTime dateTime = m_startDateTime.toLocalTime().addDays(startX);
            textDateTime = QString("%1 %2")
                               .arg(dateTime.toString("yyyy/MM/dd"))
                               .arg(MyQt::weekString(dateTime.date().dayOfWeek()));
            break;
        }
        case MonthlyReport: {
            textDateTime = QString("%1")
                               .arg(m_startDateTime.toLocalTime().addDays(startX).toString("yyyy/MM/dd"));
            break;
        }
        default:
            break;
        }

        stream << textDateTime << ",";
        for (int i = 0; i < lineData.lines.size(); ++i) {
            int lineIndex = lineData.lines.at(i);
            const QMap<int, int> &valueMap = lineData.lineData.value(lineIndex);

            int yValue = valueMap.value(j, 0);
            stream << QString("%1").arg(yValue) << ",";
        }
        stream << "\r\n";
        startX++;
    }

    return 0;
}

void PeopleCountingBackup::drawCSV(QPainter *painter, int groupFilter, const QList<int> &channels)
{
    const PeopleGridData &gridData = gPeopleCountingData.peopleLineGridData(groupFilter);
    const PeopleLineData &lineData = gPeopleCountingData.peopleLineData(groupFilter);

    QRectF gridRc(QPoint(1, 1), painter->viewport().size());
    gridRc.setRight(gridRc.right() - 2);
    gridRc.setBottom(gridRc.bottom() - 2);

    //第一列占2 * perWidth
    int columnCount = 13;
    qreal perWidth = gridRc.width() / columnCount;

    int maxNameRowCount = 1;
    //QMap<int, QList<QString>> namesMap = calHorizontalNames(painter, perWidth, channels, &maxNameRowCount);
    QMap<int, QList<QString>> namesMap;
    for (int i = 0; i < channels.size(); ++i) {
        int ch = channels.at(i);
        if (ch < 0) {
            continue;
        }
        QString name = QString("CH%1").arg(ch + 1, 2, 10, QLatin1Char('0'));
        QList<QString> list;
        list << name;
        namesMap.insert(ch, list);
    }

    int rowCount = 31 + maxNameRowCount;
    qreal perHeight = gridRc.height() / rowCount;

    //网格
    {
        //横线
        qreal x1 = gridRc.left();
        qreal y1 = gridRc.top();
        qreal x2 = gridRc.right();
        qreal y2 = y1;
        for (int row = 0; row < gridData.xLineCount + 2; ++row) {
            painter->drawLine(x1, y1, x2, y2);
            if (row == 0) {
                y1 += perHeight * maxNameRowCount;
                y2 = y1;
            } else {
                y1 += perHeight;
                y2 = y1;
            }
        }
        //竖线
        x1 = gridRc.left();
        y1 = gridRc.top();
        x2 = x1;
        y2 = gridRc.top() + (gridData.xLineCount + maxNameRowCount) * perHeight;
        for (int column = 0; column < columnCount; ++column) {
            painter->drawLine(x1, y1, x2, y2);
            if (column == 0) {
                x1 += perWidth * 2;
                x2 = x1;
            } else {
                x1 += perWidth;
                x2 = x1;
            }
        }
    }

    //horizontal header
    {
        qreal left = gridRc.left();
        qreal top = gridRc.top();
        painter->drawText(QRectF(left, top, perWidth * 2, perHeight * maxNameRowCount), Qt::AlignCenter, "Time");
        left += perWidth * 2;
        //
        QString totalText;
        switch (m_statisticsType) {
        case PeopleEntered:
            totalText = "Total Entered";
            break;
        case PeopleExited:
            totalText = "Total Exited";
            break;
        case PeopleAll:
            totalText = "Total Sum";
            break;
        default:
            break;
        }
        painter->drawText(QRectF(left, top, perWidth, perHeight * maxNameRowCount), Qt::AlignCenter, totalText);
        left += perWidth;
        //
        for (auto iter = namesMap.constBegin(); iter != namesMap.constEnd(); ++iter) {
            const QList<QString> &names = iter.value();
            if (names.size() == 1) {
                painter->drawText(QRectF(left, top, perWidth, perHeight * maxNameRowCount), Qt::AlignCenter, names.first());
            } else {
                qreal subTop = top;
                for (int i = 0; i < names.size(); ++i) {
                    painter->drawText(QRectF(left, subTop, perWidth, perHeight), Qt::AlignCenter, names.at(i));
                    subTop += perHeight;
                }
            }
            //
            left += perWidth;
        }
    }

    //vertical header
    {
        qreal left = gridRc.left();
        qreal top = gridRc.top();
        top += perHeight * maxNameRowCount;
        //
        int startX = gridData.xMinValue;
        for (int i = 0; i < gridData.xLineCount; ++i) {
            QString textDateTime;
            switch (m_recordType) {
            case DailyReport:
                textDateTime = QString("%1 %2:00-%3:00")
                                   .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                   .arg(startX, 2, 10, QLatin1Char('0'))
                                   .arg(startX + 1, 2, 10, QLatin1Char('0'));
                break;
            case WeeklyReport:
                textDateTime = QString("%1 %2")
                                   .arg(m_startDateTime.toLocalTime().toString("yyyy/MM/dd"))
                                   .arg(MyQt::weekString(startX + 1));
                break;
            case MonthlyReport:
                textDateTime = QString("%1")
                                   .arg(m_startDateTime.toLocalTime().addDays(startX).toString("yyyy/MM/dd"));
                break;
            default:
                break;
            }
            QTextOption textOption(Qt::AlignCenter);
            textOption.setWrapMode(QTextOption::WrapAnywhere);
            painter->drawText(QRectF(left, top, perWidth * 2, perHeight), textDateTime, textOption);
            //
            top += perHeight;
            startX++;
        }
    }

    //value
    qreal left = gridRc.left();
    left += perWidth * 2;
    qreal top = gridRc.top();
    top += perHeight * maxNameRowCount;
    for (int j = 0; j < gridData.xLineCount; ++j) {
        for (int i = 0; i < lineData.lines.size(); ++i) {
            int lineIndex = lineData.lines.at(i);
            const QMap<int, int> &valueMap = lineData.lineData.value(lineIndex);

            int yValue = valueMap.value(j, 0);
            //
            painter->drawText(QRectF(left, top, perWidth, perHeight), Qt::AlignCenter, QString("%1").arg(yValue));
            //
            left += perWidth;
        }
        //
        left = gridRc.left() + perWidth * 2;
        top += perHeight;
    }
}

int PeopleCountingBackup::backupPDF(const QString &filePath, int groupFilter, const QList<int> &channels)
{
    int result = 0;

    QPrinter printer;
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);
    printer.setOutputFileName(filePath);
    printer.setOutputFormat(QPrinter::PdfFormat);

    QPainter pdfPainter(&printer);
    pdfPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //
    QRect imageRc(QPoint(0, 0), m_viewportGeometry.size());
    QRect pdfRc = pdfPainter.viewport();
    qreal r1 = (qreal)imageRc.width() / imageRc.height();
    qreal r2 = (qreal)pdfRc.width() / pdfRc.height();
    if (r1 > r2) {
        imageRc.setWidth(pdfRc.width());
        imageRc.setHeight(pdfRc.width() / r1);
    } else {
        imageRc.setWidth(pdfRc.height() * r1);
        imageRc.setHeight(pdfRc.height());
    }
    imageRc.moveCenter(pdfRc.center());

    //
    {
        QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
        image.fill(Qt::white);

        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing);

        switch (m_chartMode) {
        case LineChart: {
            LineScene *scene = new LineScene;
            scene->setSceneRect(m_viewportGeometry);
            scene->setMainTitle(m_mainTitle);
            scene->setSubTitle(m_subTitleForBackup + m_groupNameMap.value(groupFilter));
            scene->showLine(channels, groupFilter);
            scene->render(&painter);
            scene->deleteLater();
            break;
        }
        case HistogramChart: {
            HistogramScene *scene = new HistogramScene;
            scene->setSceneRect(m_viewportGeometry);
            scene->setMainTitle(m_mainTitle);
            scene->setSubTitle(m_subTitleForBackup + m_groupNameMap.value(groupFilter));
            scene->showHistogram(channels, groupFilter);
            scene->render(&painter);
            scene->deleteLater();
            break;
        }
        case Histogram2Chart: {
            HistogramScene *scene = new HistogramScene;
            scene->setSceneRect(m_viewportGeometry);
            scene->setMainTitle(m_mainTitle);
            scene->setSubTitle(m_subTitleForBackup + m_groupNameMap.value(groupFilter));
            scene->showHistogram2(channels, groupFilter);
            scene->render(&painter);
            scene->deleteLater();
            break;
        }
        default:
            break;
        }

        //
        pdfPainter.drawImage(imageRc, image);
    }

    //
    printer.newPage();

    //
    {
        QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
        image.fill(Qt::white);

        QPainter painter(&image);

        drawCSV(&painter, groupFilter, channels);

        //
        pdfPainter.drawImage(imageRc, image);
    }

    return result;
}

int PeopleCountingBackup::backupPNG(const QString &filePath, int groupFilter, const QList<int> &channels)
{
    int result = 0;

    QImage image(m_viewportGeometry.size(), QImage::Format_RGB888);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing);

    switch (m_chartMode) {
    case LineChart: {
        LineScene *scene = new LineScene;
        scene->setSceneRect(m_viewportGeometry);
        scene->setMainTitle(m_mainTitle);
        scene->setSubTitle(m_subTitleForBackup + m_groupNameMap.value(groupFilter));
        scene->showLine(channels, groupFilter);
        scene->render(&painter);
        scene->deleteLater();
        break;
    }
    case HistogramChart: {
        HistogramScene *scene = new HistogramScene;
        scene->setSceneRect(m_viewportGeometry);
        scene->setMainTitle(m_mainTitle);
        scene->setSubTitle(m_subTitleForBackup + m_groupNameMap.value(groupFilter));
        scene->showHistogram(channels, groupFilter);
        scene->render(&painter);
        scene->deleteLater();
        break;
    }
    case Histogram2Chart: {
        HistogramScene *scene = new HistogramScene;
        scene->setSceneRect(m_viewportGeometry);
        scene->setMainTitle(m_mainTitle);
        scene->setSubTitle(m_subTitleForBackup + m_groupNameMap.value(groupFilter));
        scene->showHistogram2(channels, groupFilter);
        scene->render(&painter);
        scene->deleteLater();
        break;
    }
    default:
        break;
    }

    bool ok = image.save(filePath);
    if (!ok) {
        qMsWarning() << "save image error:" << filePath;
        result = -1;
    }

    return result;
}

QMap<int, QList<QString>> PeopleCountingBackup::calHorizontalNames(QPainter *painter, qreal perWidth, const QList<int> &channels, int *maxRowCount)
{
    QMap<int, QList<QString>> namesMap;
    for (int i = 0; i < channels.size(); ++i) {
        int channel = channels.at(i);
        if (channel < 0) {
            continue;
        }
        QString channelName = qMsNvr->channelName(channel);
        QList<QString> &names = namesMap[channel];
        //名字太长，分割
        QFontMetrics metrics(painter->font());
        int width = 0;
        int pos = 0;
        for (int i = 0; i < channelName.size(); ++i) {
            width += metrics.width(channelName.at(i));
            if (width > perWidth) {
                names << channelName.mid(pos, i - pos);
                width = 0;
                pos = i;
            }
        }
        if (names.isEmpty()) {
            names << channelName;
        }
        *maxRowCount = qMax(*maxRowCount, names.size());
    }
    return namesMap;
}

void PeopleCountingBackup::setLineMask(int lineMask)
{
    m_lineMask = lineMask;
}

void PeopleCountingBackup::setTextMap(const QMap<int, QString> &textMap)
{
    m_textMap = textMap;
}

void PeopleCountingBackup::setChannels(const QList<int> &channels)
{
    m_channels = channels;
}
