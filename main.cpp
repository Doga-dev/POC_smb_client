/*!
 * \file %{Cpp:License:FileName}
 * \brief file for the definition of the class "%{Cpp:License:ClassName}"
 * \author doga
 * \date 2024-10-10
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
#include <QGuiApplication>
#include <QQmlApplicationEngine>

extern int main1(int argc, char * argv[]);
extern int main2();
extern int main3(int argc, char *argv[]);


int main(int argc, char * argv[])
{
    // return main2();
    return main3(argc, argv);
}
