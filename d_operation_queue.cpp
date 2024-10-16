/*!
 * \file d_operation_queue.cpp
 * \brief file for the definition of the class "DOperationQueue"
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
#include "d_operation_queue.h"

DOperationQueue::DOperationQueue(QObject * parent)
    : QObject{parent}
    , m_queue()
    , m_isProcessing(false)
{}

void DOperationQueue::enqueue(std::function<void()> operation)
{
    m_queue.enqueue(operation);
    if (! m_isProcessing) {
        processNextOperation();
    }
}

void DOperationQueue::processNextOperation()
{
    if (m_queue.isEmpty()) {
        m_isProcessing = false;
        return;
    }

    m_isProcessing = true;
    auto operation = m_queue.dequeue();
    operation();

    QTimer::singleShot(0, this, & DOperationQueue::processNextOperation);
}
