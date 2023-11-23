//Declare libraries
#include "main.h"
#include "Lcd.h"
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"
#include "math.h"

//Define LCD Pins
#define LCD_DISPLAY_BLINK   0x0F
//Declare variables
bool CBHT = false, run = false, stop = false, set_mode, ctrl_mode, auto_mode, man_mode;
volatile int16_t adc1_value, adc2_value;
volatile float pos1, pos2;
char receivedData_A[100], receivedData_B[100];
uint8_t dataIndex_A = 0, dataIndex_B = 0;
char Rx_indx, Rx_Buffer[50],Rx_data[2];
volatile float cas_a, cas_b, F1, F2; //Value of  weighing indicator A, B
volatile int16_t  count = 1;
int count_s, count_b = 0;
uint32_t cycles = 0;
//LCD variables
int value = 1;
int mode = 0;
int auto_man = 0;
int run_stop = 0;
int air1 = 0, air2 = 0;

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DAC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_UART4_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM3_Init(void);

//PID Parameter struct
typedef struct {
    volatile float e_k;
    volatile float e_k_1;
    volatile float MV_pid_k;
    volatile float MV_pid_k_1;
    float K_c;
    float tau_i;
    float DesiredForce;
    volatile float pv;
    volatile int16_t x;
    int x_min;
    int x_max;
    int y_max;
    int y_min;
    float delta_t;
    uint32_t dac_channel;
} PIDController;

//Main funtions
void LCD_Update(void);
void Mode(void);
void Set_Start_Stop(void);
void Tare(void);
void Up(void);
void Set_Ctrl(void);
void PID_Control(PIDController *pid);
void Manual_A(void);
void Manual_B(void);

PIDController PID_A = {
    .e_k_1 = 0,
    .e_k = 0,
    .MV_pid_k_1 = 0,
    .MV_pid_k = 0,
    .K_c = 0.07,
    .tau_i = 1.75,
    .DesiredForce = 0,
    .pv = 0,
    .x = 0,
    .x_min = 2048,
    .x_max = 4095,
    .y_max = 100,
    .y_min = 0,
    .delta_t = 0.5,
	.dac_channel = DAC_CHANNEL_1
};

PIDController PID_B = {
    .e_k_1 = 0,
    .e_k = 0,
    .MV_pid_k_1 = 0,
    .MV_pid_k = 0,
    .K_c = 0.15,
    .tau_i = 1.75,
    .DesiredForce = 0,
    .pv = 0,
    .x = 0,
    .x_min = 2048,
    .x_max = 4095,
    .y_max = 100,
    .y_min = 0,
    .delta_t = 0.5,
	.dac_channel = DAC_CHANNEL_2
};

// PID Function
void PID_Control(PIDController *pid) {
    pid->e_k_1 = pid->e_k;
    pid->e_k = pid->DesiredForce - pid->pv;
    pid->MV_pid_k_1 = pid->MV_pid_k;
    pid->MV_pid_k = pid->MV_pid_k_1 + pid->K_c * (1 + (pid->delta_t / pid->tau_i)) * pid->e_k - pid->K_c * pid->e_k_1;

    if (pid->MV_pid_k > 100.0) {
        pid->MV_pid_k = 100.0;
    }
    if (pid->MV_pid_k < 0.0) {
        pid->MV_pid_k = 0.0;
    }

    pid->x = (int16_t)(((pid->MV_pid_k - pid->y_min) / (pid->y_max - pid->y_min)) * (pid->x_max - pid->x_min) + pid->x_min);

    // Assuming hdac is a global variable accessible here
    HAL_DAC_SetValue(&hdac, pid->dac_channel, DAC_ALIGN_12B_R, pid->x);
    HAL_Delay(500);
}

// Button handling functions
void Set_Ctrl(void)
{
	if (HAL_GPIO_ReadPin(GPIOE, DI4_Pin) == 0)
	{
		set_mode = true;
		ctrl_mode = false;
		LCD_Clear();
		LCD_PrintString(0, 4, "Settings");
		HAL_Delay(1000);
		LCD_Clear();
		LCD_PrintString(0, 0, "Select Parameter");
	}

	else if (HAL_GPIO_ReadPin(GPIOE, DI5_Pin) == 0)
	{
		set_mode = false;
		ctrl_mode = true;
		LCD_Clear();
		LCD_PrintString(0, 3, "Controller");
		HAL_Delay(1000);
		LCD_Clear();
		LCD_PrintString(0, 0, "Select Mode");
		LCD_PrintString(1, 1, "Auto or Manual");
	}
}

void Mode(void)
{
	if (HAL_GPIO_ReadPin(GPIOE, DI0_Pin) == 0)
	{
		HAL_Delay(100);
		if (HAL_GPIO_ReadPin(GPIOE, DI0_Pin) == 0)
			{
				if (set_mode == 1)
				{
					mode ++;
					if (mode > 5) mode = 1;
					if (mode == 1)
					{
						value = PID_A.DesiredForce;
						LCD_Clear();
						LCD_Update();
						LCD_PrintString(0, 0, "Force A");
						if (PID_A.DesiredForce != 0)
						{
							LCD_PrintNum( 1, 10, PID_A.DesiredForce);
						}
					}
					else if (mode == 2)
					{
						value = PID_B.DesiredForce;
						LCD_Clear();
						LCD_Update();
						LCD_PrintString(0, 0, "Force B");
						if (PID_B.DesiredForce != 0)
						{
							LCD_PrintNum( 1, 10, PID_B.DesiredForce);
						}
					}
					else if (mode == 3)
					{
						value = cycles;
						LCD_Clear();
						LCD_Update();
						LCD_PrintString(0, 0, "Cycles");
						if (cycles != 0)
						{
							LCD_PrintNum( 1, 10, cycles);
						}
					}
					else if (mode == 4)
					{
						value = air1;
						LCD_Clear();
						LCD_Update();
						LCD_PrintString(0, 0, "Air A");
						if (air1 != 0)
						{
							LCD_PrintNum( 1, 10, air1);
						}
					}
					else if (mode == 5)
					{
						value = air2;
						LCD_Clear();
						LCD_Update();
						LCD_PrintString(0, 0, "Air B");
						if (air2 != 0)
						{
							LCD_PrintNum( 1, 10, air2);
						}
					}
				}
				else if (ctrl_mode == 1)
				{
					auto_man++;
					if (auto_man > 2) auto_man = 1;
					if (auto_man == 1)
					{
						auto_mode = true;
						man_mode = false;
						LCD_Clear();
						LCD_PrintString(0, 0, "Auto Mode");
						HAL_Delay(300);
						LCD_PrintString(1, 0, "State: ");
					}
					else if (auto_man == 2)
					{
						auto_mode = false;
						man_mode = true;
						LCD_Clear();
						LCD_PrintString(0, 0, "Manual Mode");
						HAL_Delay(300);
						LCD_PrintString(1, 0, "State: ");
					}
				}
			}
	}
}

void Set_Start_Stop(void)
{
	if (HAL_GPIO_ReadPin(GPIOE, DI1_Pin) == 0)
	{
		HAL_Delay(100);
		if(HAL_GPIO_ReadPin(GPIOE, DI1_Pin) == 0)
		{

			if (set_mode == 1)
			{
				switch(mode)
				{
				case 1:
					PID_A.DesiredForce = value;
					printf("%.2f""d\r\n", PID_A.DesiredForce);
					LCD_PrintString(0, 14, "OK");
					break;
				case 2:
					PID_B.DesiredForce = value;
					printf("%.2f""e\r\n", PID_B.DesiredForce);
					LCD_PrintString(0, 14, "OK");
					break;
				case 3:
					cycles = value;
					printf("%d""f\r\n", cycles);
					LCD_PrintString(0, 14, "OK");
					break;
				case 4:
					if (value > 500)
					{
						LCD_PrintString(0, 10, "FALIED");
						while(HAL_GPIO_ReadPin(GPIOE, DI3_Pin) == 1)
						{
							LCD_PrintNum( 1, 10, value);
							HAL_Delay(500);
							LCD_PrintString(1, 10, "       M");
							HAL_Delay(500);
						}
					}
					else
					{
						air1 = value;
						printf("%d""g\r\n", air1);
						LCD_PrintString(0, 10, "      M");
						LCD_PrintString(0, 14, "OK");
					}
					break;
				case 5:
					if (value > 500)
					{
						LCD_PrintString(0, 10, "FALIED");
						while(HAL_GPIO_ReadPin(GPIOE, DI3_Pin) == 1)
						{
							LCD_PrintNum( 1, 10, value);
							HAL_Delay(500);
							LCD_PrintString(1, 10, "       M");
							HAL_Delay(500);
						}
					}
					else
					{
						air2 = value;
						printf("%d""h\r\n", air2);
						LCD_PrintString(0, 10, "      M");
						LCD_PrintString(0, 14, "OK");
					}
				default:
					break;
				}
			}
			else if (ctrl_mode == 1)
			{
				if (auto_man == 1)
				{
					run_stop++;
					if (run_stop > 2) run_stop = 1;
					if (run_stop == 1)
					{
						run = true;
						LCD_Clear();
						LCD_PrintString(0, 0, "Cycles:");
						LCD_PrintNum(0, 10, count);
						LCD_PrintString(1, 0, "State:   RUNNING");
					}
					else if (run_stop == 2)
					{
						run = false;
						LCD_Clear();
						LCD_PrintString(0, 2, "MACHINE STOP");
						LCD_PrintString(1, 0, "State:   WARNING");
					}
				}
			}
		}
	}
}


void Manual_A(void)
{
	if (HAL_GPIO_ReadPin(GPIOE, DI2_Pin) == 0)
		{
			HAL_Delay(100);
			if(HAL_GPIO_ReadPin(GPIOE, DI2_Pin) == 0)
			{
				if (ctrl_mode == 1)
				{
					if (man_mode == 1)
					{
						count_s++;
						if (count_s > 2) count_s = 1;
						if (count_s == 1)
						{
							LCD_PrintString(1, 10, "A+");
						}
						else if ((count_s == 2) && (PID_B.pv < PID_B.DesiredForce))
						{
							LCD_PrintString(1, 10, "A-");
						}
					}
				}
			}
		}
}

void Tare(void)
{
	if (HAL_GPIO_ReadPin(GPIOE, DI2_Pin) == 0)
		{
			HAL_Delay(100);
			if(HAL_GPIO_ReadPin(GPIOE, DI2_Pin) == 0)
			{
				if(set_mode == 1)
				{
					value = value*10;
					switch(mode)
					{
					case 1:
						if (value > 9999) value = 0;
						break;
					case 2:
						if (value > 999) value = 0;
						break;
					case 3:
						if (value > 999999) value = 0;
						break;
					case 4:
						if (value > 500) value = 0;
						break;
					case 5:
						if (value > 500) value = 0;
						break;
					}
					LCD_Update();
				}
			}
		}
}

void Manual_B(void)
{
	if (HAL_GPIO_ReadPin(GPIOE, DI3_Pin) == 0)
		{
			HAL_Delay(100);
			if (HAL_GPIO_ReadPin(GPIOE, DI3_Pin) == 0)
			{

				if (ctrl_mode == 1)
				{
					if (man_mode == 1)
					{
						count_b++;
						if (count_b > 2) count_b = 1;
						if ((count_b == 1) && (PID_A.pv >= PID_A.DesiredForce))
						{
							LCD_PrintString(1, 14, "B+");
						}
						else if (count_b == 2)
						{
							LCD_PrintString(1, 14, "B-");
						}
					}
				}
			}
		}
}

void Up(void)
{
	if (HAL_GPIO_ReadPin(GPIOE, DI3_Pin) == 0)
	{
		HAL_Delay(100);
		if (HAL_GPIO_ReadPin(GPIOE, DI3_Pin) == 0)
		{
			if (set_mode == 1)
			{
			  int digitValue = value % 10;
			  digitValue++;
			  if (digitValue > 9){
				  digitValue = 0;
			  }
			  value = (value/10)*10 + digitValue;
			  LCD_Update();
			}
		}
	}
}

void LCD_Update(void)
{
	  LCD_PrintString(1, 0, "Value: ");
	  LCD_PrintNum( 1, 10, value);
	  printf("%d\r\n", value);
}


//Transfer data to F205
#ifdef __GNUC__
	#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
	#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
	#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif
	PUTCHAR_PROTOTYPE
	{
		HAL_UART_Transmit(&huart1, (uint8_t*)&ch,1,100);
		return ch;
	}


void ProcessData_A()
{
	char tempData_A[100];
	strncpy(tempData_A, receivedData_A, dataIndex_A);
	strtok(tempData_A, ",");
	strtok(NULL, ",");
	strtok(NULL, ",");
	cas_a = atof(strtok(NULL, ","));
	F1 = cas_a*9.6545;
	PID_A.pv = F1;
	memset(receivedData_A, 0, sizeof(receivedData_A));
	dataIndex_A = 0;
}

void ProcessData_B()
{
	char tempData_B[100];
	strncpy(tempData_B, receivedData_B, dataIndex_B);
	strtok(tempData_B, ",");
	strtok(NULL, ",");
	strtok(NULL, ",");
	cas_b = atof(strtok(NULL, ","));
	F2 = cas_b*9.6545;
	PID_B.pv = F2;
	memset(receivedData_B, 0, sizeof(receivedData_B));
	dataIndex_B = 0;
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		uint8_t i;
		//Receive data from F205
		if(huart->Instance == USART1) //uart1
		{
			if(Rx_indx==0) {for (i=0;i<50;i++) Rx_Buffer[i] = 0;}
			if(Rx_data[0] != 13)
			{
				Rx_Buffer[Rx_indx++] = Rx_data[0];
			}
			switch(Rx_data[0])
			{
				case 'e':
					run = false;
					count = 0;
					LCD_Clear();
					LCD_PrintString(0, 2, "MACHINE STOP");
					LCD_PrintString(1, 0, "State:   WARNING");
					break;
				case 'r':
					run = true;
					LCD_Clear();
					LCD_PrintString(0, 0, "Cycles:");
					LCD_PrintNum(0, 10, count);
					LCD_PrintString(1, 0, "State:   RUNNING");
					break;
				case 'a':
					PID_A.DesiredForce = atof(Rx_Buffer);
					memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
					Rx_indx = 0;
					break;
				case 'b':
					PID_B.DesiredForce = atof(Rx_Buffer);
					memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
					Rx_indx = 0;
					break;
				case 'f':
					cycles = atoi(Rx_Buffer);
					memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
					Rx_indx = 0;
					break;
				default:
					break;
			}
			HAL_UART_Receive_IT(&huart1,(uint8_t*)Rx_data,1);
		}

		//Receive data from weighing indicator
		if (huart->Instance == huart3.Instance) //Check Port
		{
			  if (strncmp(&receivedData_A[dataIndex_A - 2], "\r\n", 2) == 0)
			  {
			    ProcessData_A();
			  }
			  else
			  {
			    receivedData_A[dataIndex_A++] = huart->Instance->DR;
			  }
			  HAL_UART_Receive_IT(&huart3, (uint8_t *)&receivedData_A[dataIndex_A], 1);
		}

		if (huart->Instance == huart4.Instance) // Check Port 3
		{
			  if (strncmp(&receivedData_B[dataIndex_B - 2], "\r\n", 2) == 0)
			  {
			    ProcessData_B();
			  }
			  else
			  {
			    receivedData_B[dataIndex_B++] = huart->Instance->DR;
			  }
			  HAL_UART_Receive_IT(&huart4, (uint8_t *)&receivedData_B[dataIndex_B], 1);
		}
	}


//Read analog data (Linear position sensor)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == hadc1.Instance) // check interrupt of ADC 1
	{
		adc1_value = HAL_ADC_GetValue(&hadc1);
		pos1 = 750.0 - (float)(adc1_value*750)/4095;
	}

	if (hadc->Instance == hadc2.Instance) // check interrupt of ADC 2
	{
		adc2_value = HAL_ADC_GetValue(&hadc2);
		pos2 = 600.0 - (float)(adc2_value*600)/4095;
	}
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==htim3.Instance) //TIMER3 transfer data to STM32F205
	{
		printf("%.2f""a\r\n", F1);
		printf("%.2f""b\r\n", F2);
		printf("%d""c\r\n", count);
		printf("%.2f""d\r\n", PID_A.DesiredForce);
		printf("%.2f""e\r\n", PID_B.DesiredForce);
		printf("%d""f\r\n", cycles);
		printf("%d""k\r\n", run);
		printf("q%.2f\r\n", F1);
		printf("w%.2f\r\n", F2);
		printf("i%.2f\r\n", pos1);
		printf("y%.2f\r\n", pos2);
		printf("u%d\r\n", count);
	}

	if(htim->Instance==htim4.Instance) //TIMER4 indicator light display
	{
		  if (run == 1)
		  {
			  HAL_GPIO_WritePin(GPIOD, DO10_Pin, 1);
			  HAL_GPIO_WritePin(GPIOB, DO11_Pin, 0);
		  }
		  else
		  {
			  HAL_GPIO_TogglePin(GPIOD, DO10_Pin);
			  HAL_GPIO_WritePin(GPIOB, DO11_Pin, 1);
		  }
	}
}
//External interrupt for stop button and cruise sensor
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == DI6_Pin) //cruise sensor
	{
		CBHT = 1;
		run = 0;
	}
	else if (GPIO_Pin == DI1_Pin) //stop button
	{
		run = 0;
		count = 0;
	}
}


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DAC_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_UART4_Init();
  MX_USART3_UART_Init();
  MX_ADC2_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();

  LCD_Init();
  HAL_GPIO_WritePin(LCDLED_GPIO_Port,LCDLED_Pin,1);
  HAL_ADC_Start_IT(&hadc1);
  HAL_ADC_Start_IT(&hadc2);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_UART_Receive_IT(&huart1,(uint8_t*)Rx_data,1);
  HAL_UART_Receive_IT(&huart3, (uint8_t *)&receivedData_A[dataIndex_A], 1);
  HAL_UART_Receive_IT(&huart4, (uint8_t *)&receivedData_B[dataIndex_B], 1);
  LCD_Clear();
  LCD_PrintString(0, 2, "Select Mode!");
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1638);
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 1638);
  HAL_Delay(2000);

  while (1)
  {
	 Set_Ctrl();
	 Mode();
	 Set_Start_Stop();
	 Tare();
	 Up();
	 if (CBHT == 0)
	  {
		 if (auto_mode == 1)
		 {
			if (run == 1)
			{
			  while (count < cycles)
				{
					while (PID_A.pv < PID_A.DesiredForce)
					{
						PID_Control(&PID_A);
						if (run == 0) break;
					}
					while (PID_B.pv < PID_B.DesiredForce)
					{
						PID_Control(&PID_B);
						if (run == 0) break;
					}
					while (pos2 > 290.1)
					{
						HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 1230);
					}
					HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 2048);
					HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1638);
					count++;
					LCD_PrintString(0, 0, "Cycles:");
					LCD_PrintNum(0, 10, count);
					HAL_Delay(500);
					if (run == 0) break;
				}
			  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 1230);
			  HAL_Delay(100);
			  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1638);
			  if (count == cycles)
			  {
				  LCD_Clear();
				  LCD_PrintString(0, 0, "TEST COMPLETTED!");
				  HAL_Delay(500);
			  }
			}
			else
			{
			  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 1230);
			  HAL_Delay(100);
			  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1638);
			}
		 }
		 else if (man_mode == 1)
		 {
			 Manual_A();
			 Manual_B();
			if (count_s == 1)
			{
				PID_Control(&PID_A);
				Manual_A();
				Manual_B();
			}
			else if ((count_s == 2) && (PID_B.pv < PID_B.DesiredForce))
			{
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1638);
			}
			if ((count_b == 1) && (PID_A.pv >= PID_A.DesiredForce))
			{
				PID_Control(&PID_B);
				Manual_B();
			}
			else if (count_b == 2)
			{
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 1638);
			}
		 }
	  }
	else
	{
	  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1230);
	  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 1638);
	}
  }
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_ADC2_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_DAC_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 16800;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 199;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 16800;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 2999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_UART4_Init(void)
{
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOE, Buzzer_Pin|DE2_RE2_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOA, DE3_RE3_Pin|DO0_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(DO11_GPIO_Port, DO11_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(DO10_GPIO_Port, DO10_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPIOD, DO9_Pin|DO8_Pin|DO7_Pin|DO6_Pin
                          |DO5_Pin|LCDD4_Pin|LCDD5_Pin|LCDD6_Pin
                          |LCDD7_Pin|LCDLED_Pin|LCDEN_Pin|LCDRW_Pin
                          |LCDRS_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOC, DO4_Pin|DO3_Pin|DO2_Pin|DO1_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, BT_CLK_Pin|LED_DATA_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = Buzzer_Pin|DE2_RE2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DI13_Pin|DI12_Pin|DI11_Pin|DI10_Pin
                          |DI9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DE3_RE3_Pin|DO0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DI8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DI8_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DI7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DI7_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DI6_Pin|DI1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DI5_Pin|DI4_Pin|DI3_Pin|DI2_Pin
                          |DI0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DO11_Pin|BT_CLK_Pin|LED_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DO10_Pin|DO9_Pin|DO8_Pin|DO7_Pin
                          |DO6_Pin|DO5_Pin|LCDD4_Pin|LCDD5_Pin
                          |LCDD6_Pin|LCDD7_Pin|LCDLED_Pin|LCDEN_Pin
                          |LCDRW_Pin|LCDRS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DO4_Pin|DO3_Pin|DO2_Pin|DO1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif
