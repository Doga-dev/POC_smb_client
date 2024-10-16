/*!
 * \file d_operation_queue.h
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
#ifndef D_OPERATION_QUEUE_H
#define D_OPERATION_QUEUE_H

#include <QObject>
#include <QQueue>
#include <QTimer>

class DOperationQueue : public QObject
{
    Q_OBJECT

public:
    explicit    DOperationQueue         (QObject * parent = nullptr);

    void        enqueue                 (std::function<void()> operation);

private:
    void        processNextOperation    ();

    QQueue<std::function<void()>> m_queue;
    bool                          m_isProcessing;
};

#endif // D_OPERATION_QUEUE_H
