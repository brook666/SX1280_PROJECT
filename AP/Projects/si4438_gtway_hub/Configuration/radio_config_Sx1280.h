


#define RADIO_NSS_PIN       GPIO_Pin_4
#define RADIO_NSS_PORT      GPIOA

#define RADIO_MOSI_PIN      GPIO_Pin_7
#define RADIO_MOSI_PORT     GPIOA

#define RADIO_MISO_PIN      GPIO_Pin_6
#define RADIO_MISO_PORT     GPIOA

#define RADIO_SCK_PIN       GPIO_Pin_5
#define RADIO_SCK_PORT      GPIOA

#define RADIO_nRESET_PIN    GPIO_Pin_0
#define RADIO_nRESET_PORT   GPIOA

#define RADIO_BUSY_PIN      GPIO_Pin_3
#define RADIO_BUSY_PORT     GPIOB

#define RADIO_DIOx_PIN      GPIO_Pin_10
#define RADIO_DIOx_PORT     GPIOC

#define USART_TX_PIN        GPIO_Pin_2
#define USART_TX_PORT       GPIOA

#define USART_RX_PIN        GPIO_Pin_3
#define USART_RX_PORT       GPIOA

#define ANT_SW_PIN          GPIO_Pin_0
#define ANT_SW_PORT         GPIOB

#define LED_RX_PIN          GPIO_Pin_10
#define LED_RX_PORT         GPIOB

#define LED_TX_PIN          GPIO_Pin_10
#define LED_TX_PORT         GPIOB



/*!
 * \brief Defines the RF mode
 */
#define MODE_LORA

/*!
 * \brief Defines the nominal frequency
 */

#define RF_FREQUENCY                                2400000000U// Hz

/*!
 * \brief Defines the output power in dBm
 *
 * \remark The range of the output power is [-18..+13] dBm
 */
#define TX_OUTPUT_POWER                             13

/*!
 * \brief Defines the buffer size, i.e. the payload size
 */
#define BUFFER_SIZE                                 249


/*!
 * \brief Number of tick size steps for rx timeout
 */
#define RX_TIMEOUT_VALUE                            0xFFFF // ms

/*!
 * \brief Number of tick size steps for rx timeout
 */
#define TX_TIMEOUT_VALUE                            4000 // ms


/*!
 * \brief Size of ticks (used for Tx and Rx timeout)
 */
#define RX_TIMEOUT_TICK_SIZE                        RADIO_TICK_SIZE_1000_US


