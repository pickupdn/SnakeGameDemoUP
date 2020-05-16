/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include <stdio.h>
//#include <time.h>
//#include <stdlib.h>
//#include<stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define width 80
#define height 25
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// -> y-axis  horicontal  ,    v x-axis vertical
int gameSpeed = 5;
int isGameOver;
int headX, headY, powerX, powerY, score, nTail = 1;
int headX2, headY2;
int tailX[width * height]; // array of x-axis of tail
int tailY[width * height]; // array of y-axis of tail
enum eDirection { //direction of head
	STOP = 0, LEFT, RIGHT, UP, DOWN
};
enum eDirection dir;
uint8_t field[100][100]; // 0 space, 1 tailsnake, 2 headsnake, 3 power
uint8_t inputText[1];
uint8_t space[] = " "; // 0
uint8_t tail[] = "O"; // 1
uint8_t head[] = "H"; // 2
uint8_t headleft[] = "<"; // 2 left
uint8_t headup[] = "^"; // 2 up
uint8_t headright[] = ">"; // 2 right
uint8_t headdown[] = "V"; // 2 down
uint8_t power[] = "P"; // 3
uint8_t left[] = "DIRECTION : LEFT \r\n";
uint8_t up[] = "DIRECTION : UP   \r\n";
uint8_t right[] = "DIRECTION : RIGHT\r\n";
uint8_t down[] = "DIRECTION : DOWN \r\n";

void setCurcor(int x, int y); //assign position of cursor
void repositionPower(); //reposition of power
void setup(); // setup game befor start
void drawEdge(); // draw a frame
void updateTail(int x, int y); // update position of tail
void drawSnake(); // draw head and tail of snake
void checkPosition(); // check that is head through the frame
void checkEatPower();
void checkEatTail();
void move();
void printScore();
void printDir();
void printPosition();
void printSpeed();
void printDes();
void printIns();

void setCurcor(int x, int y) { // start 0,0
	x++, y++;
	int size = 6;
	if (x >= 10)
		size++;
	if (y >= 10)
		size++;
	char command[15];
	sprintf(command, "%s%d%c%d%c", "\033[", x, ';', y, 'H');
	HAL_UART_Transmit(&huart2, command, size, HAL_MAX_DELAY);
}

void repositionPower() {
	powerY = (rand() % width) + 1;
	powerX = (rand() % height) + 1;
	if (field[powerX][powerY] != 0)
		repositionPower();
	field[powerX][powerY] = 3;
	setCurcor(powerX, powerY);
	HAL_UART_Transmit(&huart2, power, sizeof(power), HAL_MAX_DELAY);
}

void setup() {
	isGameOver = 0;
	dir = STOP;
	headX = height / 2;
	headY = width / 2;
	field[headX][headY] = 2;
	repositionPower();
	printIns();
}

void drawEdge() {
	setCurcor(0, 0);
	uint8_t horizonEdge[width + 5] = "";
	for (int i = 0; i <= width + 1; i++)
		strcat(horizonEdge, "-");
	strcat(horizonEdge, "\n\r");
	HAL_UART_Transmit(&huart2, horizonEdge, sizeof(horizonEdge), HAL_MAX_DELAY);
	uint8_t verticalEdge[width + 5] = "";
	strcat(verticalEdge, "|");
	for (int i = 1; i <= width; i++)
		strcat(verticalEdge, " ");
	strcat(verticalEdge, "|\r\n");
	for (int i = 1; i <= height; i++) {
		HAL_UART_Transmit(&huart2, verticalEdge, width + 5, HAL_MAX_DELAY);
	}
	HAL_UART_Transmit(&huart2, horizonEdge, width + 5, HAL_MAX_DELAY);
}

void updateTail(int x, int y) {
	int tempX1 = tailX[0];
	int tempY1 = tailY[0];
	int tempX2, tempY2;
	tailX[0] = x;
	tailY[0] = y;
	for (int i = 1; i <= nTail; i++) {
		tempX2 = tailX[i];
		tempY2 = tailY[i];
		tailX[i] = tempX1;
		tailY[i] = tempY1;
		tempX1 = tempX2;
		tempY1 = tempY2;
	}
	field[tailX[0]][tailY[0]] = 1;
	field[tempX2][tempY2] = 0;
	setCurcor(tempX2, tempY2);
	HAL_UART_Transmit(&huart2, space, sizeof(space), HAL_MAX_DELAY);

}

void drawSnake() {
	for (int x = 1; x <= height; x++) {
		for (int y = 1; y <= width; y++) {
			if (field[x][y] == 1) {
				setCurcor(x, y);
				HAL_UART_Transmit(&huart2, tail, sizeof(tail), HAL_MAX_DELAY);
			} else if (field[x][y] == 2) {
				setCurcor(x, y);
				switch (dir) {
				case LEFT:
					HAL_UART_Transmit(&huart2, headleft, sizeof(headleft),
					HAL_MAX_DELAY);
					break;
				case UP:
					HAL_UART_Transmit(&huart2, headup, sizeof(headup),
					HAL_MAX_DELAY);
					break;
				case RIGHT:
					HAL_UART_Transmit(&huart2, headright, sizeof(headright),
					HAL_MAX_DELAY);
					break;
				case DOWN:
					HAL_UART_Transmit(&huart2, headdown, sizeof(headdown),
					HAL_MAX_DELAY);
					break;
				default:
					break;
				}
			}
		}
	}
}

void checkPosition() {
	if (headY >= width + 1)
		headY = 1;
	else if (headY <= 0)
		headY = width;
	if (headX >= height + 1)
		headX = 1;
	else if (headX <= 0)
		headX = height;
}

void checkEatPower() {//
	if (field[headX][headY] == 3) {
		nTail++;
		score++;
		printScore();
		repositionPower();
		gameSpeed += 1;
		printSpeed();
	}
}

void checkEatTail() {
	if (dir != STOP && field[headX][headY] == 1)
		isGameOver = 1;
}

void move() {
	headX2 = headX;
	headY2 = headY;
	switch (dir) {
	case LEFT:
		headY--;
		break;
	case UP:
		headX--;
		break;
	case RIGHT:
		headY++;
		break;
	case DOWN:
		headX++;
		break;
	default:
		break;
	}
	checkPosition();
	printPosition();
	checkEatPower();
	checkEatTail();
	field[headX][headY] = 2;
	updateTail(headX2, headY2);
	drawSnake();

}

void printScore() {
	setCurcor(27, 0);
	uint8_t temp[16] = "";
	sprintf(temp, " SCORE = %3d\r\n", score);
	HAL_UART_Transmit(&huart2, temp, sizeof(temp), HAL_MAX_DELAY);
}

void printDir() {
	switch (dir) {
	case LEFT:
		setCurcor(28, 0);
		HAL_UART_Transmit(&huart2, left, sizeof(left), HAL_MAX_DELAY);
		break;
	case UP:
		setCurcor(28, 0);
		HAL_UART_Transmit(&huart2, up, sizeof(up), HAL_MAX_DELAY);
		break;
	case RIGHT:
		setCurcor(28, 0);
		HAL_UART_Transmit(&huart2, right, sizeof(right), HAL_MAX_DELAY);
		break;
	case DOWN:
		setCurcor(28, 0);
		HAL_UART_Transmit(&huart2, down, sizeof(down), HAL_MAX_DELAY);
		break;
	default:
		break;
	}
}

void printPosition() {
	uint8_t positionHead[19] = "";
	sprintf(positionHead, " Y = %2d  X = %2d\r\n", headY, headX);
	setCurcor(29, 0);
	HAL_UART_Transmit(&huart2, positionHead, sizeof(positionHead),
	HAL_MAX_DELAY);
	uint8_t positionPower[19] = "";
	sprintf(positionPower, "pY = %2d pX = %2d\r\n", powerY, powerX);
	setCurcor(30, 0);
	HAL_UART_Transmit(&huart2, positionPower, sizeof(positionPower),
	HAL_MAX_DELAY);
}

void printSpeed() {
	uint8_t speed[19] = "";
	sprintf(speed, " SPEED = %3d\r\n", gameSpeed);
	setCurcor(31, 0);
	HAL_UART_Transmit(&huart2, speed, sizeof(speed),
	HAL_MAX_DELAY);
}

void printDes() {
	printScore();
	printDir();
	printPosition();
	printSpeed();
}

void printIns() {
	uint8_t l[] = "a = LEFT";
	setCurcor(27, 60);
	HAL_UART_Transmit(&huart2, l, sizeof(l),
	HAL_MAX_DELAY);
	uint8_t u[] = "w = UP";
	setCurcor(28, 60);
	HAL_UART_Transmit(&huart2, u, sizeof(u),
	HAL_MAX_DELAY);
	uint8_t r[] = "d = RIGHT";
	setCurcor(29, 60);
	HAL_UART_Transmit(&huart2, r, sizeof(r),
	HAL_MAX_DELAY);
	uint8_t d[] = "s = DOWN";
	setCurcor(30, 60);
	HAL_UART_Transmit(&huart2, d, sizeof(d),
	HAL_MAX_DELAY);
	uint8_t key[] = "Please choose any direction to start";
	setCurcor(31, 40);
	HAL_UART_Transmit(&huart2, key, sizeof(key),
	HAL_MAX_DELAY);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	/* Prevent unused argument(s) compilation warning */
	UNUSED(huart);
	/* NOTE : This function should not be modified, when the callback is needed,
	 the HAL_UART_RxCpltCallback can be implemented in the user file
	 */
	switch (*inputText) {
	case 'a':
		if (dir == RIGHT)
			break;
		dir = LEFT;
		printDir();
		break;
	case 'w':
		if (dir == DOWN)
			break;
		dir = UP;
		printDir();
		break;
	case 'd':
		if (dir == LEFT)
			break;
		dir = RIGHT;
		printDir();
		break;
	case 's':
		if (dir == UP)
			break;
		dir = DOWN;
		printDir();
		break;
	default:
		break;
	}
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	/* USER CODE BEGIN 2 */
	HAL_UART_Receive_DMA(&huart2, inputText, sizeof(inputText));
	srand((unsigned) (time(NULL)));
	drawEdge();
	setup();
	printDes();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (!isGameOver) {
		/* USER CODE END WHILE */
		move();
		if (dir == LEFT || dir == RIGHT)
			HAL_Delay(300 / gameSpeed);
		if (dir == UP || dir == DOWN)
			HAL_Delay(800 / gameSpeed);
		/* USER CODE BEGIN 3 */
	}
	printDes();
	uint8_t gameOverText[] = "Game Over !!!\r\n";
	setCurcor(height / 4, width / 2 - 10);
	HAL_UART_Transmit(&huart2, gameOverText, sizeof(gameOverText),
	HAL_MAX_DELAY);
	/* USER CODE END 3 */
}

/** -----------------------------------------------------------------------------------
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

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
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/** 
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LD2_Pin */
	GPIO_InitStruct.Pin = LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
