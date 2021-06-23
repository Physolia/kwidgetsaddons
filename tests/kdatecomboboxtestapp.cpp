/*
    SPDX-FileCopyrightText: 2021 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdatecombobox.h"
#include <QApplication>
#include <QDate>
#include <QDebug>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QApplication app{argc, argv};
    KDateComboBox dateComboBox;
    dateComboBox.setOptions(KDateComboBox::EditDate
                            | KDateComboBox::SelectDate
                            | KDateComboBox::DatePicker
                            | KDateComboBox::DateKeywords
                            | KDateComboBox::WarnOnInvalid);
    dateComboBox.setDateRange(QDate{2021, 1, 1}, QDate{2106, 2, 6});
    QObject::connect(&dateComboBox, &KDateComboBox::dateEntered, [](const QDate &d) { qDebug() << "dateEntered" << d; });
    QObject::connect(&dateComboBox, &KDateComboBox::dateChanged, [](const QDate &d) { qDebug() << "dateChanged" << d; });
    QObject::connect(&dateComboBox, &KDateComboBox::dateEdited, [](const QDate &d) { qDebug() << "dateEdited" << d; });
    dateComboBox.resize(200, dateComboBox.sizeHint().height());
    dateComboBox.show();
    // dateComboBox.setEnabled(false);
    return app.exec();
}
