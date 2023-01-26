#ifndef TRANSFER_RUN_H
#define TRANSFER_RUN_H

/*! \file run.h
 *  \brief Определение фукнции run
 */

#include "service.h"

namespace transfer{

    /*! \brief Обеспечивает работу классов sender и receiver\n
     * Функция предполагает использование после всех посылок и получений объектов\n
     * Функция не будет завершена, пока все данные не будут отправлены/получены
     */

    void run();
}


#endif
