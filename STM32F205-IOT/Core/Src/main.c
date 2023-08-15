// KHAI BAO THU VIEN
#include "main.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

// HAM GUI DU LIEU GIAO TIEP STM32F407
#ifdef __GNUC__
     #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
     #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart2,(uint8_t *)&ch,1,0xFFFF);
	return ch;
}

// KHAI BAO BIEN JSON
//NHAN DU LIEU
cJSON *str_json, *str_ON , *str_SetFA , *str_SetairA , *str_SetFB  ,*str_SetairB,*str_SetC, *str_Mode,*str_FAF407,*str_FBF407,*str_CF407,*str_run;

//TRUYEN DU LIEU
float aFA,bFB,FAF407,FBF407;
unsigned int cCycle,CycleF407,run;

// SETTING AIRA, AIRB
float airA,airB;

// KHAI BAO BIEN TRUYEN
uint8_t  rx_index, rx_data;
char rx_buffer[100];

// KHAI BAO BIEN KET NOI VOI ESP8266
uint8_t  rx_index1, rx_data1;
char rx_buffer1[500];
char ResponseRX[500];

// KHAI BAO BIEN DOC GIA TRI QUA CONG COM VA UART2
char rx_buffer2[50],rx_data2[2];
uint8_t rx_index2;

// KHAI BAO CAC BIEN PHAN HOI
uint32_t  rx_indexResponse;
uint8_t ErrorCode = 0;
int ConfigAT = 0;
long last = 0;
uint8_t CheckConnect = 1;

// KHAI BAO BIEN CHINH CUA HE THONG
unsigned int ON;


// KHAI BAO SERVER MQTT

char *mqtt_server = "broker.hivemq.com";
char *mqtt_port = "1883";
char *mqtt_user = "trangnguyenIOT";
char *mqtt_pass = "123456nnn";
char *mqtt_sub = "trangnguyen/tr"; // NHAN DU LIEU
char *mqtt_pub = "trangnguyen/sub"; // TRUYEN DU LIEU

// CAC HAM CHINH CUA HE THONG

void SettingESP(void);
void clearbuffer_UART_ESP(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void Send_AT_Commands_Setting(char *AT_Commands, char *DataResponse, uint32_t timesend , uint32_t setting);
void Received_AT_Commands_ESP(void);
void Received_AT_Commands_ESP_MessagerMQTT(void);
void clearResponse(void);
void ConnectMQTT(char *server , char *port , char *user , char *pass , char *sub , char *pub);
void Send_AT_Commands_ConnectMQTT(char *AT_Commands, char *DataResponse , uint32_t timeout , uint32_t setting , uint32_t count);
void Send_AT_Commands_SendMessager(char *AT_Commands, char *DataResponse , uint32_t timeout , uint32_t setting , uint32_t count);
void Send_AT_Commands_CheckConnectMQTT(char *AT_Commands, char *DataResponse , uint32_t timeout , uint32_t setting , uint32_t count);
void SendData(char *pub );
void ParseJson(char *DataMQTT);
void SendMQTT(void);

// VIET CAC HAM CON
void SendMQTT(void)
{
	while(1)
	{
	if(HAL_GetTick() - last >= 500)
		{
			if(CheckConnect)
			{
				SendData( mqtt_pub );
			}
			last = HAL_GetTick();
			break;
		}
	}
}

// GUI DU LIEU KIEU INT SANG CHAR
void SendData(char *pub )
{
	char MQTTPUBRAW[100];
	char JSON[100];
	char Length[100];
	char Str_FA[100];
	char Str_FB[100];
	char Str_C[100];
	char Str_FAF407[100];
	char Str_FBF407[100];
	char Str_CF407[100];
	char Str_run[100];
	for(int i = 0 ; i < 100; i++)
	{
		MQTTPUBRAW[i] = 0;
		JSON[i] = 0;
		Length[i] = 0;
		Str_FA[i]=0;
		Str_FB[i]=0;
		Str_C[i]=0;
		Str_FAF407[i]=0;
		Str_FBF407[i]=0;
		Str_CF407[i]=0;
		Str_run[i]=0;
	}
	sprintf(Str_FA, "%.2f", aFA);
	sprintf(Str_FB, "%.2f", bFB);
	sprintf(Str_C, "%d", cCycle);
	sprintf(Str_FAF407, "%.2f", FAF407);
	sprintf(Str_FBF407, "%.2f", FBF407);
	sprintf(Str_CF407, "%d", CycleF407);
	sprintf(Str_run, "%d", run);

	// TAO CHUOI JSON GUI LEN MQTT
	strcat(JSON,"{\"FA\":\"");
	strcat(JSON,Str_FA);
	strcat(JSON,"\",");
	strcat(JSON,"\"FB\":\"");
	strcat(JSON,Str_FB);
	strcat(JSON,"\",");
	strcat(JSON,"\"C\":\"");
	strcat(JSON,Str_C);
	strcat(JSON,"\",");
	strcat(JSON,"\"FAF407\":\"");
	strcat(JSON,Str_FAF407);
	strcat(JSON,"\",");
	strcat(JSON,"\"FBF407\":\"");
	strcat(JSON,Str_FBF407);
	strcat(JSON,"\",");
	strcat(JSON,"\"CF407\":\"");
	strcat(JSON, Str_CF407);
	strcat(JSON,"\",");
	strcat(JSON,"\"run\":\"");
	strcat(JSON,Str_run);
	strcat(JSON,"\"}");
	strcat(JSON,"\r\n");
	int len = 0;
	len = strlen(JSON);
	sprintf(Length, "%d", len);
	strcat(MQTTPUBRAW,"AT+MQTTPUBRAW=0,\"");
	strcat(MQTTPUBRAW,pub);
	strcat(MQTTPUBRAW,"\",");
	strcat(MQTTPUBRAW,Length);
	strcat(MQTTPUBRAW,",0,1\r\n");

	// CONTINUE CHECK CONNECT
	Send_AT_Commands_SendMessager(MQTTPUBRAW, "OK\r\n\r\n>" , 5000 , 0 , 3);
	clearbuffer_UART_ESP();
	if(ErrorCode == 0)
	{
		Send_AT_Commands_SendMessager(JSON, "+MQTTPUB:OK" , 5000 , 0 , 5);
		clearbuffer_UART_ESP();
	}
	ConfigAT = 1;
}

// CONNECT MQTT
void ConnectMQTT(char *server , char *port , char *user , char *pass , char *sub , char *pub)
{
	uint32_t id = 0;
	id = rand()%100;
	char clientid[100];
	char MathRandom[100];
	char MQTTUSERCFG[100];
	char MQTTCONN[100];
	char MQTTSUB[100];
	for(int i = 0 ; i < 100; i++)
	{
		clientid[i] = 0;
		MathRandom[i] = 0;
		MQTTUSERCFG[i] = 0;
		MQTTCONN[i] = 0;
		MQTTSUB[i] = 0;
	}
	printf(MathRandom, "%d", id);
	strcat(clientid, "ESP");
	strcat (clientid, MathRandom);

	// TAO CHUOI USER AND PASS
	strcat(MQTTUSERCFG, "AT+MQTTUSERCFG=0,1,\"");
	strcat(MQTTUSERCFG,clientid);
	strcat(MQTTUSERCFG,"\",\"");
	strcat(MQTTUSERCFG,user);
	strcat(MQTTUSERCFG,"\",\"");
	strcat(MQTTUSERCFG,pass);
	strcat(MQTTUSERCFG,"\",0,0,");
	strcat(MQTTUSERCFG,"\"\"");
	strcat(MQTTUSERCFG,"\r\n");

	// TAO CHUOI SERVER AND PORT
	strcat(MQTTCONN, "AT+MQTTCONN=0,\"");
	strcat(MQTTCONN, server);
	strcat(MQTTCONN, "\",");
	strcat(MQTTCONN, port);
	strcat(MQTTCONN, ",1\r\n");

	// TAO CHUOI DE NHAN DU LIEU
	strcat(MQTTSUB, "AT+MQTTSUB=0,\"");
	strcat(MQTTSUB, sub);
	strcat(MQTTSUB, "\",0\r\n");

	// GUI LENH QUA ESP8266 DE KET NOI WIFI
	Send_AT_Commands_ConnectMQTT(MQTTUSERCFG, "OK" , 5000 , 0 , 5);
	HAL_Delay(1000);
	clearbuffer_UART_ESP();
	Send_AT_Commands_ConnectMQTT(MQTTCONN, "+MQTTCONNECTED" , 5000 , 0 , 5);
	HAL_Delay(1000);
	clearbuffer_UART_ESP();
	Send_AT_Commands_ConnectMQTT(MQTTSUB, "OK" , 5000 , 0 , 5);
	HAL_Delay(1000);
	clearbuffer_UART_ESP();
	ConfigAT = 1;
	ErrorCode = 1;
}

// GUI KET NOI DEN ESP8266
void Send_AT_Commands_SendMessager(char *AT_Commands, char *DataResponse , uint32_t timeout , uint32_t setting , uint32_t count)
{
	clearbuffer_UART_ESP();
	last = HAL_GetTick();
	uint32_t Size = 300;
	uint32_t Count = 0;
	ConfigAT = setting;
	char DataHTTP[Size];
	for(int i = 0 ; i < Size; i++)
	{
		DataHTTP[i] = 0;
	}

	// GUI LENH VAO HTTP
	snprintf(DataHTTP, sizeof(DataHTTP),"%s", AT_Commands);
	HAL_UART_Transmit(&huart1,(uint8_t *)&DataHTTP,strlen(DataHTTP),1000);
	last = HAL_GetTick();
	while(1)
	{
		// CHAY HAM NGAT UART

		if(HAL_GetTick() - last >= timeout)
		{
			Count++;
			HAL_UART_Transmit(&huart1,(uint8_t *)&DataHTTP,strlen(DataHTTP),1000);

			//printf("Send AT-Commands Send Data MQTT: %s\r\n", DataHTTP);

			last = HAL_GetTick();
		}

		if(strstr(rx_buffer1,DataResponse) != NULL)
		{

			clearbuffer_UART_ESP();
			ErrorCode = 0;
			CheckConnect = 1;
			last = HAL_GetTick();
			break;
		}
		if(Count >= count)
		{

			ErrorCode = 1;
			clearbuffer_UART_ESP();
			last = HAL_GetTick();
			break;
		}
	}
}

void Send_AT_Commands_ConnectMQTT(char *AT_Commands, char *DataResponse , uint32_t timeout , uint32_t setting , uint32_t count)
{
	clearbuffer_UART_ESP();
	last = HAL_GetTick();
	uint32_t Size = 300;
	uint32_t Count = 0;
	ConfigAT = setting;
	char DataHTTP[Size];
	for(int i = 0 ; i < Size; i++)
	{
		DataHTTP[i] = 0;
	}

	// GUI LENH VAO HTTP
	snprintf(DataHTTP, sizeof(DataHTTP),"%s", AT_Commands);
	HAL_UART_Transmit(&huart1,(uint8_t *)&DataHTTP,strlen(DataHTTP),1000);
	last = HAL_GetTick();
	while(1)
	{
		// CHAY HAM NGAT UART

		if(HAL_GetTick() - last >= timeout)
		{
			Count++;
			HAL_UART_Transmit(&huart1,(uint8_t *)&DataHTTP,strlen(DataHTTP),1000);
			last = HAL_GetTick();

		}
		if(strstr(rx_buffer1,DataResponse) != NULL)
		{

			clearbuffer_UART_ESP();
			ErrorCode = 1;
			CheckConnect = 1;
			last = HAL_GetTick();
			break;
		}
		if(Count >= count)
		{

			//GUI LAI LENH SETTING

			ErrorCode = 0;
			CheckConnect = 0;
			clearbuffer_UART_ESP();
			last = HAL_GetTick();
			break;
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// ConfigAT = 0

	if(ConfigAT == 0)
	{
		if(huart -> Instance == USART1)
		{
			Received_AT_Commands_ESP();
			HAL_UART_Receive_IT(&huart1,&rx_data1,1);
		}
	}

	// ConfigAT = 1

	else if(ConfigAT == 1)
	{
		if(huart -> Instance == USART1)
		{
			Received_AT_Commands_ESP_MessagerMQTT();
			HAL_UART_Receive_IT(&huart1,&rx_data1,1);
		}
	}
	uint8_t i;
			if(huart->Instance == USART2) //uart1
			{

				if(rx_index2==0) {for (i=0;i<50;i++) rx_buffer2[i] = 0;}
				if(rx_data2[0] != 13)
				{
					rx_buffer2[rx_index2++] |= rx_data2[0];
				}
				switch(rx_data2[0])
				{
					case 'a':
						aFA = atof(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'b':
						bFB = atof(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'c':
						cCycle = atoi(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'd':
						FAF407 = atof(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'e':
						FBF407 = atof(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'f':
						CycleF407 = atoi(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'g':
						airA = atof(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'h':
						airB = atof(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					case 'k':
						run = atoi(rx_buffer2);
						memset(rx_buffer2, 0, sizeof(rx_buffer2));
						rx_index2 = 0;
						break;
					default:
						break;
				}
				HAL_UART_Receive_IT(&huart2, (uint8_t*)rx_data2, 1);
			}
}

void Received_AT_Commands_ESP(void)
{
	rx_buffer1[rx_index1++] = rx_data1;
}

void Received_AT_Commands_ESP_MessagerMQTT(void)
{

		if(rx_data1 != '\n')
		{
			ResponseRX[rx_indexResponse++] = rx_data1;
		}
		else
		{
			ResponseRX[rx_indexResponse++] = rx_data1;
			rx_indexResponse = 0;

			// CHECK CONNECT

			if(strstr(ResponseRX,"MQTTCONNECTED") != NULL)
			{

				CheckConnect = 1;
				last = HAL_GetTick();
			}
			else if(strstr(ResponseRX,"MQTTDISCONNECTED") != NULL)
			{

				CheckConnect = 0;
				last = HAL_GetTick();
			}
			else if(strstr(ResponseRX,"+MQTTSUBRECV") != NULL)
			{

				char *DataMQTT;
				DataMQTT = strtok(ResponseRX,",");
				DataMQTT = strtok(NULL,",");
				DataMQTT = strtok(NULL,",");
				DataMQTT = strtok(NULL,"\n");
				ParseJson(DataMQTT);
				last = HAL_GetTick();
			}
			last = HAL_GetTick();
			clearResponse();
		}
}

void ParseJson(char *DataMQTT)
{
	str_json = cJSON_Parse(DataMQTT);
	if (!str_json)
	{

		return;
	}
	else
	{
	      // JSON OK
	       str_SetFA = cJSON_GetObjectItem(str_json, "SetFA");
	        if (str_SetFA != NULL)
	        {
	            printf("%s""a\r\n\n", str_SetFA->valuestring);
	        }
			str_SetFB = cJSON_GetObjectItem(str_json, "SetFB");
	        if (str_SetFB != NULL)
	        {
	            printf("%s""b\r\n", str_SetFB->valuestring);
	        }

	        str_SetairA = cJSON_GetObjectItem(str_json, "SetairA");
	        if (str_SetairA != NULL)
	        {
	            airA=atof(str_SetairA->valuestring);
	        }
	        str_SetairB = cJSON_GetObjectItem(str_json, "SetairB");
	        if (str_SetairB != NULL)
	        {
	        	airB=atof(str_SetairB->valuestring);
	        }

	      str_SetC = cJSON_GetObjectItem(str_json, "SetC");
	        if (str_SetC != NULL)
	        {
	            printf("%s""f\r\n", str_SetC->valuestring);
	        }

	        str_ON = cJSON_GetObjectItem(str_json, "ON");
	        if (str_ON->type == cJSON_String)
	        {
	        	if(strstr(str_ON->valuestring,"1") != NULL)
	        	{

				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_RESET);
				ON = atoi(str_ON->valuestring);
				if(ON==1)
				{
					printf("r\r\n");
				}
			}
			if(strstr(str_ON->valuestring,"0") != NULL)
			{

				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_SET);
				ON = atoi(str_ON->valuestring);
				if(ON==0)
				{
					printf("e\r\n");
				}
			}
		}

			cJSON_Delete(str_json);
	}
}


void clearbuffer_UART_ESP(void)
{
	for(int i = 0 ; i < 500 ; i++)
	{
		rx_buffer1[i] = 0;
	}
	rx_index1 = 0;
}


void clearResponse(void)
{
	for(int i = 0 ; i < 500; i++)
	{
		ResponseRX[i] = 0;
	}
	rx_indexResponse = 0;
}

void Send_AT_Commands_Setting(char *AT_Commands, char *DataResponse, uint32_t timesend , uint32_t setting)
{
	last = HAL_GetTick();
	ConfigAT = setting;
	char DataSendAT[50];
	for(int i = 0 ; i < 50; i++)
	{
		DataSendAT[i] = 0;
	}
	snprintf(DataSendAT, sizeof(DataSendAT),"%s\r\n", AT_Commands);

	// GUI DU LIEU TU ESP8266 QUA CONG COM 1 UART1
	HAL_UART_Transmit(&huart1,(uint8_t *)&DataSendAT,strlen(DataSendAT),1000);

	// DUNG DOI PHAN HOI, DOI LAU QUA THI HOI TIEP(THONG QUA HAM NGAT UART)
	last = HAL_GetTick();
	while(1)
	{

		// QUA 5S THI GUI LAI LENH CU, OK THI THOI

		if(HAL_GetTick() - last >= timesend)
		{
			HAL_UART_Transmit(&huart1,(uint8_t *)&DataSendAT,strlen(DataSendAT),1000);

			last = HAL_GetTick();
		}
		if(strstr(rx_buffer1,DataResponse) != NULL)
		{

			clearbuffer_UART_ESP();
			break;
		}
	}
}

// HAM SETTING DATA
void SettingESP(void)
{

	// RESET ESP8266
	Send_AT_Commands_Setting("AT+RST\r\n", "OK", 10000 , 0);
	HAL_Delay(3000);

	// ESP CO HOAT DONG KHONG
	Send_AT_Commands_Setting("AT\r\n", "OK", 300, 0);
	HAL_Delay(3000);

	// TAT KH CAN PHAN HOI
	Send_AT_Commands_Setting("ATE0\r\n", "OK" , 2000, 0);
	HAL_Delay(3000);

	// CAI DAT CHE DO HOAT DONG 1,1
	Send_AT_Commands_Setting("AT+CWMODE=1,1\r\n", "OK", 2000, 0);
	HAL_Delay(3000);

	// WIFI CAN KET NOI
	Send_AT_Commands_Setting("AT+CWJAP=\"HOPELESS\",\"nhatnguyeniot\"\r\n", "WIFI CONNECTED", 10000, 0);
	HAL_Delay(3000);
	Send_AT_Commands_Setting("AT+CIPMUX=0\r\n", "OK", 2000 , 0);
	HAL_Delay(3000);
	ErrorCode = 0;
}


int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_UART4_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_DAC_Init();
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
	HAL_Delay(1200);
	HAL_UART_Receive_IT(&huart1, &rx_data1, 1);
	HAL_UART_Receive_IT(&huart2, (uint8_t*)rx_data2, 1);
	HAL_Delay(3000);
	SettingESP();
	HAL_Delay(3000);
	ConnectMQTT(mqtt_server , mqtt_port , mqtt_user , mqtt_pass, mqtt_sub , mqtt_pub);
	last = HAL_GetTick();

// VONG LAP TUAN HOAN
  while (1)
  {
	  SendMQTT();
	  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, airA*4095/500);
	  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, airB*4095/500);
	  HAL_Delay(300);
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 13;
  RCC_OscInitStruct.PLL.PLLN = 195;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
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
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE5
                           PE6 PE12 PE13 PE14
                           PE15 PE0 PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC0 PC1 PC2
                           PC3 PC4 PC5 PC6
                           PC7 PC8 PC9 PC10
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA6 PA7 PA8 PA11
                           PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB12 PB13 PB14
                           PB15 PB3 PB4 PB5
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PE7 PE8 PE9 PE10
                           PE11 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 PD10 PD11
                           PD12 PD13 PD14 PD15
                           PD0 PD1 PD2 PD3
                           PD4 PD5 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
