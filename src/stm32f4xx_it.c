/* USER CODE BEGIN Header */

#include "main.h"
#include "stm32f4xx_it.h"

extern TIM_HandleTypeDef htim2;

void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }

}

void HardFault_Handler(void)
{

  while (1)
  {
  }
}

void MemManage_Handler(void)
{

  while (1)
  {

  }
}

void BusFault_Handler(void)
{

  while (1)
  {

  }
}

void UsageFault_Handler(void)
{

  while (1)
  {
 
  }
}

void SVC_Handler(void)
{

}
void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
  
}

void SysTick_Handler(void)
{

  HAL_IncTick();

}

void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void TIM2_IRQHandler(void)
{

  HAL_TIM_IRQHandler(&htim2);

}

