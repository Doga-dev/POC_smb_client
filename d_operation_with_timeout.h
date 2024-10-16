/*!
 * \file d_operation_with_timeout.h
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
#ifndef D_OPERATION_WITH_TIMEOUT_H
#define D_OPERATION_WITH_TIMEOUT_H

#include <QFuture>
#include <QObject>
#include <QTimer>

class DOperationWithTimeout : public QObject
{
    Q_OBJECT

public:
    explicit DOperationWithTimeout(std::function<void(std::function<void()>)> operation,
                                   int                                        timeoutMs,
                                   QObject *                                  parent = nullptr);

public slots:
    void        resetTimer              ();

signals:
    void        requestStartTimer       ();

    void        operationCompleted      ();
    void        timeoutOccurred         ();

private slots:
    void        onTimeout               ();

private:
    void        startTimer              ();

    std::function<void(std::function<void()>)>  m_operation;
    QTimer                                      m_timer;
    int                                         m_timeoutMs;
    QFuture<void>                               m_future;
};

#endif // D_OPERATION_WITH_TIMEOUT_H
