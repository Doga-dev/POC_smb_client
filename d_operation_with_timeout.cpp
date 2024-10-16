/*!
 * \file d_operation_with_timeout.cpp
 * \brief file for the definition of the class "DOperationWithTimeout"
 * \author doga
 * \date 2024-10-15
 *
 * \details
 *
 * \copyright
 ****************************************************************************
 *        Ce logiciel est la propriete de DOGA®.
 *         -------------------------------------
 *
 *    Il ne peut etre reproduit ni communique en totalite ou partie sans
 *    son autorisation ecrite.
 *
 ****************************************************************************
 *        This software is the property of DOGA®.
 *         -------------------------------------
 *
 *    It cannot be reproduced nor disclosed entirely or partially without
 *    a written agreement.
 *
 ****************************************************************************
 */

#include <QtConcurrent/QtConcurrent>

#include "d_operation_with_timeout.h"

DOperationWithTimeout::DOperationWithTimeout(std::function<void(std::function<void()>)> operation,
                                             int                                        timeoutMs,
                                             QObject *                                  parent)
    : QObject(parent)
    , m_operation(operation)
    , m_timer(this)
    , m_timeoutMs(timeoutMs)
    , m_future()
{
    m_timer.setSingleShot(true);
    connect(& m_timer, & QTimer::timeout, this, & DOperationWithTimeout::onTimeout);
    connect(this, &DOperationWithTimeout::requestStartTimer, this, &DOperationWithTimeout::startTimer, Qt::QueuedConnection);
    startTimer();

    m_future = QtConcurrent::run([this]() {
        m_operation([this]() { this->resetTimer(); });
        emit operationCompleted();
    });
}

void DOperationWithTimeout::resetTimer()
{
    emit requestStartTimer();
}

void DOperationWithTimeout::onTimeout()
{
    if (m_future.isRunning()) {
        emit timeoutOccurred();
    }
}

void DOperationWithTimeout::startTimer()
{
    if (m_timer.isActive()) {
        m_timer.stop();
    }
    m_timer.start(m_timeoutMs);
}
