/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktogglefullscreenaction.h"

#include <QEvent>

class Q_DECL_HIDDEN KToggleFullScreenAction::Private
{
public:
    Private(KToggleFullScreenAction *action)
        : q(action)
    {
    }

    void updateTextsAndIcon()
    {
        if (q->isChecked()) {
            q->setText(KToggleFullScreenAction::tr("Exit F&ull Screen Mode", "@action:inmenu"));
            q->setIconText(KToggleFullScreenAction::tr("Exit Full Screen", "@action:intoolbar"));
            q->setToolTip(KToggleFullScreenAction::tr("Exit full screen mode", "@info:tooltip"));
            q->setIcon(QIcon::fromTheme(QStringLiteral("view-restore")));
        } else {
            q->setText(KToggleFullScreenAction::tr("F&ull Screen Mode", "@action:inmenu"));
            q->setIconText(KToggleFullScreenAction::tr("Full Screen", "@action:intoolbar"));
            q->setToolTip(KToggleFullScreenAction::tr("Display the window in full screen", "@info:tooltip"));
            q->setIcon(QIcon::fromTheme(QStringLiteral("view-fullscreen")));
        }
    }

    KToggleFullScreenAction *const q;
    QWidget *window = nullptr;
};

KToggleFullScreenAction::KToggleFullScreenAction(QObject *parent)
    : KToggleAction(parent),
      d(new Private(this))
{
    d->updateTextsAndIcon();
}

KToggleFullScreenAction::KToggleFullScreenAction(QWidget *window, QObject *parent)
    : KToggleAction(parent),
      d(new Private(this))
{
    d->updateTextsAndIcon();
    setWindow(window);
}

KToggleFullScreenAction::~KToggleFullScreenAction() = default;

void KToggleFullScreenAction::setWindow(QWidget *window)
{
    if (d->window) {
        d->window->removeEventFilter(this);
    }

    d->window = window;

    if (d->window) {
        d->window->installEventFilter(this);
    }
}

void KToggleFullScreenAction::slotToggled(bool checked)
{
    KToggleAction::slotToggled(checked);
    d->updateTextsAndIcon();
}

bool KToggleFullScreenAction::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->window)
        if (event->type() == QEvent::WindowStateChange) {
            if (d->window->isFullScreen() != isChecked()) {
                activate(QAction::Trigger);
            }
        }

    return false;
}

void KToggleFullScreenAction::setFullScreen(QWidget *window, bool set)
{
    if (set) {
        window->setWindowState(window->windowState() | Qt::WindowFullScreen);
    } else {
        window->setWindowState(window->windowState() & ~Qt::WindowFullScreen);
    }
}

