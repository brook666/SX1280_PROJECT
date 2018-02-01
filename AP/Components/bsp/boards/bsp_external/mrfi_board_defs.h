#ifndef MRFI_BOARD_DEFS_H
#define MRFI_BOARD_DEFS_H

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"

//zeng add
extern uint8_t IRQ_INT_FLAG;
//zeng add end
/* ------------------------------------------------------------------------------------------------
 *                                           Defines
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                        Radio Selection
 * ------------------------------------------------------------------------------------------------
 */
#if (!defined MRFI_SI4438) && \
    (!defined MRFI_SI4463)
#error "ERROR: A compatible radio must be specified for the MSP430F149 board."
/*
 *  Since the EXP461x board can support several different radios, the installed
 *  radio must be specified with a #define.  It is best to do this at the
 *  project level.  However, if only one radio will ever be used, a #define
 *  could be placed here, above this error check.
 */
#endif


/* ------------------------------------------------------------------------------------------------
 *                                      ENTX Pin Configuration
 * ------------------------------------------------------------------------------------------------
 */
#define __mrfi_ENTX_BIT__           4
#define MRFI_ENTX_PIN_AS_OUTPUT()   //st( GPIO_InitTypeDef GPIO_InitStructure;\
                                        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);\
                                        GPIO_InitStructure.GPIO_Pin = BV(__mrfi_ENTX_BIT__);\
                                        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;\
                                        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;\
                                        GPIO_Init(GPIOA, &GPIO_InitStructure);\
                                      )
#define MRFI_ENTX_HIGH()            //st( GPIOA->ODR |=  BV(__mrfi_ENTX_BIT__); __nop();__nop();) 
#define MRFI_ENTX_LOW()            // st(  __nop();__nop();) 
#define MRFI_ENTX_IS_HIGH()           //( GPIOA->ODR &   BV(__mrfi_ENTX_BIT__) )

/* ------------------------------------------------------------------------------------------------
 *                                      ENRX Pin Configuration
 * ------------------------------------------------------------------------------------------------
 */
#define __mrfi_ENRX_BIT__           4
#define MRFI_ENRX_PIN_AS_OUTPUT()   //st( GPIO_InitTypeDef GPIO_InitStructure;\
                                        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);\
                                        GPIO_InitStructure.GPIO_Pin = BV(__mrfi_ENRX_BIT__);\
                                        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;\
                                        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;\
                                        GPIO_Init(GPIOA, &GPIO_InitStructure);\
                                      )
#define MRFI_ENRX_HIGH()           // st( GPIOA->ODR &= ~BV(__mrfi_ENRX_BIT__); __nop();__nop();) 
#define MRFI_ENRX_LOW()             //st( __nop();__nop();) 
#define MRFI_ENRX_IS_HIGH()          // ( GPIOA->ODR &   BV(__mrfi_ENRX_BIT__) )








/* ------------------------------------------------------------------------------------------------
 *                                      ENTX Pin Configuration
 * ------------------------------------------------------------------------------------------------
 */
#define __mrfi_CTS_GPIO_BIT__             5
#define MRFI_CONFIG_CTS_PIN_AS_INPUT()    st( GPIO_InitTypeDef GPIO_InitStructure;\
                                              RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);\
                                              GPIO_InitStructure.GPIO_Pin = BV(__mrfi_CTS_GPIO_BIT__);\
                                              GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;\
                                              GPIO_Init(GPIOC, &GPIO_InitStructure);)
#define MRFI_CTS_IS_HIGH()                  ( GPIOA->IDR & BV(__mrfi_GPIO1_BIT__) )
#define MRFI_CTS_IS_LOW()                   ( GPIOA->ODR & BV(__mrfi_GPIO1_BIT__)))
/* ------------------------------------------------------------------------------------------------
 *                                      IRQ Pin Configuration
 * ------------------------------------------------------------------------------------------------*/
//select input,floating with interrupt,select interrupt with rising edge only
#define __mrfi_IRQ_BIT__                     10   
#define MRFI_CONFIG_IRQ_PIN_AS_INPUT()       st( GPIO_InitTypeDef GPIO_InitStructure;\
                                                  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);\
                                                  GPIO_InitStructure.GPIO_Pin = BV(__mrfi_IRQ_BIT__);\
                                                  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;\
                                                  GPIO_Init(GPIOB, &GPIO_InitStructure);\
                                                 )
#define MRFI_IRQ_PIN_IS_HIGH()               (GPIOB->IDR & BV(__mrfi_IRQ_BIT__))
//#define MRFI_IRQ_INT_VECTOR                  ITC_IRQ_PORTC+2 
#define MRFI_ENABLE_IRQ_INT()               (EXTI->IMR |=BV(__mrfi_IRQ_BIT__))
#define MRFI_DISABLE_IRQ_INT()                (EXTI->IMR &=~BV(__mrfi_IRQ_BIT__))
#define MRFI_IRQ_INT_IS_ENABLED()            (EXTI->IMR&BV(__mrfi_IRQ_BIT__))
#define MRFI_CLEAR_IRQ_INT_FLAG()             st( IRQ_INT_FLAG=0;EXTI_ClearITPendingBit(EXTI_Line10);) 
#define MRFI_IRQ_INT_FLAG_IS_SET()           ( IRQ_INT_FLAG==1 )
#define MRFI_CONFIG_IRQ_RISING_EDGE_INT()    (EXTI->RTSR |=BV(__mrfi_IRQ_BIT__)) 
#define MRFI_CONFIG_IRQ_FALLING_EDGE_INT()   (EXTI->FTSR |=BV(__mrfi_IRQ_BIT__))


/* ------------------------------------------------------------------------------------------------
 *                                      SDN Pin Configuration
 * ------------------------------------------------------------------------------------------------
 */
#define __mrfi_SDN_GPIO_BIT__                     0
#define MRFI_CONFIG_SDN_PIN_AS_OUTPUT()       st( GPIO_InitTypeDef GPIO_InitStructure;\
                                                  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);\
                                                  GPIO_InitStructure.GPIO_Pin = BV(__mrfi_SDN_GPIO_BIT__);\
                                                  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;\
                                                  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;\
                                                  GPIO_Init(GPIOB, &GPIO_InitStructure);\
                                                 )
#define MRFI_SDN_DRIVE_HIGH()              st( GPIOB->ODR |=  BV(__mrfi_SDN_GPIO_BIT__); )
#define MRFI_SDN_DRIVE_LOW()               st( GPIOB->ODR &= ~BV(__mrfi_SDN_GPIO_BIT__); )


/* ------------------------------------------------------------------------------------------------
 *                                      SPI Configuration
 * ------------------------------------------------------------------------------------------------
 */

/* CSn Pin Configuration */
#define __mrfi_SPI_CSN_GPIO_BIT__             4
#define MRFI_SPI_CONFIG_CSN_PIN_AS_OUTPUT()   st( GPIO_InitTypeDef GPIO_InitStructure;\
                                                  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);\
                                                  GPIO_InitStructure.GPIO_Pin = BV(__mrfi_SPI_CSN_GPIO_BIT__);\
                                                  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;\
                                                  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;\
                                                  GPIO_Init(GPIOA, &GPIO_InitStructure);\
                                                )
#define MRFI_SPI_DRIVE_CSN_HIGH()             st( GPIOA->ODR |=  BV(__mrfi_SPI_CSN_GPIO_BIT__); ) 
#define MRFI_SPI_DRIVE_CSN_LOW()              st( GPIOA->ODR &= ~BV(__mrfi_SPI_CSN_GPIO_BIT__); ) 
#define MRFI_SPI_CSN_IS_HIGH()                 (  GPIOA->ODR &   BV(__mrfi_SPI_CSN_GPIO_BIT__) )


/* SCLK Pin Configuration */
#define __mrfi_SPI_SCLK_GPIO_BIT__            5
#define MRFI_SPI_DRIVE_SCLK_HIGH()            st( GPIOA->ODR |=  BV(__mrfi_SPI_SCLK_GPIO_BIT__); )
#define MRFI_SPI_DRIVE_SCLK_LOW()             st( GPIOA->ODR &= ~BV(__mrfi_SPI_SCLK_GPIO_BIT__); )


/* SI Pin Configuration */
#define __mrfi_SPI_SI_GPIO_BIT__              7
#define MRFI_SPI_DRIVE_SI_HIGH()              st( GPIOA->ODR |=  BV(__mrfi_SPI_SI_GPIO_BIT__); )
#define MRFI_SPI_DRIVE_SI_LOW()               st( GPIOA->ODR &= ~BV(__mrfi_SPI_SI_GPIO_BIT__); )

/* SO Pin Configuration */
#define __mrfi_SPI_SO_GPIO_BIT__              6       
#define MRFI_SPI_SO_IS_HIGH()                 ( GPIOA->IDR & BV(__mrfi_SPI_SO_GPIO_BIT__) )


/* read/write macros */
#define MRFI_SPI_WRITE_BYTE(x)                st( while(!(SPI1->SR & SPI_I2S_FLAG_TXE));\
                                                  SPI1->DR = x;)
#define MRFI_SPI_READ_BYTE()                  SPI1->DR       //¶ÁDR¿ÉÒÔÇå³ýRXEN×´Ì¬
#define MRFI_SPI_WAIT_DONE()                  while(!(SPI1->SR & SPI_I2S_FLAG_RXNE))


/* SPI critical section macros */
typedef bspIState_t mrfiSpiIState_t;
#define MRFI_SPI_ENTER_CRITICAL_SECTION(X)    BSP_ENTER_CRITICAL_SECTION(X)
#define MRFI_SPI_EXIT_CRITICAL_SECTION(X)     BSP_EXIT_CRITICAL_SECTION(X)


#define MRFI_SPI_IS_INITIALIZED()     (!(SPI1->CR1 & SPI_CPHA_1Edge) )


/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#ifndef BSP_BOARD_MSP430F149
//#error "ERROR: Mismatch between specified board and MRFI configuration."
#endif



/*
 *  Radio SPI Specifications
 * -----------------------------------------------
 *    Max SPI Clock     :  10 MHz
 *    Baud rate control :  f/8
 *    Data Order        :  MSB transmitted first
 *    Clock Polarity    :  low when idle
 *    Clock Phase       :  sample leading edge
 *    Work Patern       :  full duplex
 *    Slave Management  :  software
 *    Mode              :  master
 */
   // 
/* initialization macro */
#define MRFI_SPI_INIT()\
st(\
  SPI_InitTypeDef  SPI_InitStructure;\
  GPIO_InitTypeDef GPIO_InitStructure;\
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);\
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);\
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;\
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;\
  GPIO_Init(GPIOA, &GPIO_InitStructure);\
  SPI_Cmd(SPI1, DISABLE);\
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;\
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;\
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;\
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;\
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;\
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;\
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;\
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;\
  SPI_InitStructure.SPI_CRCPolynomial = 7;\
  SPI_Init(SPI1, &SPI_InitStructure);\
  SPI_Cmd(SPI1, ENABLE);\
  )



/**************************************************************************************************
 */
#endif
