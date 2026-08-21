/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TARVS_APP_H
#define __TARVS_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define ENABLE_DEBUG

#define MODULE_CMD 0
#define MODULE_USART2 1
#define MODULE_GPIO 2
/* Exported types ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Main application routine running the state machine loop
 *
 * @param   None
 * @retval  None
 */
void APP_Main(void);

#ifdef __cplusplus
}
#endif

#endif /* __TARVS_APP_H */
