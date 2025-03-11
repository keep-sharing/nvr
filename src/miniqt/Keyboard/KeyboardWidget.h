/*
 * Copyright 2009 EMBITEL (http://www.embitel.com)
 * 
 * This file is part of Virtual Keyboard Project.
 * 
 * Virtual Keyboard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation
 * 
 * Virtual Keyboard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Virtual Keyboard. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KEYBOARDWIDGET_H_
#define __KEYBOARDWIDGET_H_

#include "ui_KeyboardWidget.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QWidget>

class QSignalMapper;

class KeyboardWidget : public QWidget, public Ui::KeyboardWidget {
    Q_OBJECT

public:
    KeyboardWidget(QWidget *parent = NULL);
    ~KeyboardWidget();

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private slots:
    void on_closetoolButton_released();
    void btn_clicked(int btn);
    void on_btnCaps_toggled(bool checked);
    void on_btnShiftLeft_toggled(bool checked);
    void on_btnCtrlLeft_toggled(bool checked);
    void on_btnAltLeft_toggled(bool checked);
    void changeTextShift(bool isShift);
    void changeTextCaps(bool isCaps);
    bool checkNotTextKey(int keyId);

signals:
    void sigClose();

private:
    QPoint dragPosition;
    bool isCaps;
    bool isShift;
    bool isCtrl;
    bool isAlt;
    QSignalMapper *signalMapper;
    QElapsedTimer dragThreshold;
    QList<QToolButton *> allButtons;
};
#endif /*__KEYBOARDWIDGET_H_*/
