/*

  WS2812B CPU and memory efficient library

  Date: 28.9.2016

  Author: Martin Hubacek
  	  	  http://www.martinhubacek.cz
  	  	  @hubmartin

  Licence: MIT License

*/

#include <stdint.h>

#include "stm32f3xx_hal.h"
#include "ws2812b/ws2812b.h"
#include <stdlib.h>

// RGB Framebuffers
uint8_t frameBuffer[3*60];
uint8_t frameBuffer2[3*20];

// Helper defines
#define newColor(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) <<  8) | (b))
#define Red(c) ((uint8_t)((c >> 16) & 0xFF))
#define Green(c) ((uint8_t)((c >> 8) & 0xFF))
#define Blue(c) ((uint8_t)(c & 0xFF))


uint32_t Wheel(uint8_t WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return newColor(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return newColor(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return newColor(WheelPos * 3, 255 - WheelPos * 3, 0);
}




void visRainbow(uint8_t *frameBuffer, uint32_t frameBufferSize, uint32_t effectLength)
{
	uint32_t i;
	static uint8_t x = 0;

	x += 1;

	if(x == 256*5)
		x = 0;

	for( i = 0; i < frameBufferSize / 3; i++)
	{
		uint32_t color = Wheel(((i * 256) / effectLength + x) & 0xFF);

		frameBuffer[i*3 + 0] = color & 0xFF;
		frameBuffer[i*3 + 1] = color >> 8 & 0xFF;
		frameBuffer[i*3 + 2] = color >> 16 & 0xFF;
	}
}


void visDots(uint8_t *frameBuffer, uint32_t frameBufferSize, uint32_t random, uint32_t fadeOutFactor)
{
	uint32_t i;

	for( i = 0; i < frameBufferSize / 3; i++)
	{

		if(rand() % random == 0)
		{
			frameBuffer[i*3 + 0] = 255;
			frameBuffer[i*3 + 1] = 255;
			frameBuffer[i*3 + 2] = 255;
		}


		if(frameBuffer[i*3 + 0] > fadeOutFactor)
			frameBuffer[i*3 + 0] -= frameBuffer[i*3 + 0]/fadeOutFactor;
		else
			frameBuffer[i*3 + 0] = 0;

		if(frameBuffer[i*3 + 1] > fadeOutFactor)
			frameBuffer[i*3 + 1] -= frameBuffer[i*3 + 1]/fadeOutFactor;
		else
			frameBuffer[i*3 + 1] = 0;

		if(frameBuffer[i*3 + 2] > fadeOutFactor)
			frameBuffer[i*3 + 2] -= frameBuffer[i*3 + 2]/fadeOutFactor;
		else
			frameBuffer[i*3 + 2] = 0;
	}
}



void visHandle2()
{
	static uint32_t timestamp;

	if(HAL_GetTick() - timestamp > 10)
	{
		timestamp = HAL_GetTick();

		visRainbow(frameBuffer, sizeof(frameBuffer), 15);
		visDots(frameBuffer2, sizeof(frameBuffer2), 50, 40);
/*
		switch(visSelect)
		{
			case 0:
				visRainbow(frameBuffer, sizeof(frameBuffer), 15);
				break;


			case 1:
				visDots(frameBuffer, sizeof(frameBuffer), 50, 40);
				break;

		}*/
	}
}


void visInit()
{
	// Set output channel/pin, GPIO_PIN_0 = 0, for GPIO_PIN_5 = 5 - this has to correspond to WS2812B_PINS
	ws2812b.item[0].channel = 0;
	// Your RGB framebuffer
	ws2812b.item[0].frameBufferPointer = frameBuffer;
	// RAW size of framebuffer
	ws2812b.item[0].frameBufferSize = sizeof(frameBuffer);


	// If you need more parallel LED strips, increase the WS2812_BUFFER_COUNT value
	ws2812b.item[1].channel = 3;
	ws2812b.item[1].frameBufferPointer = frameBuffer2;
	ws2812b.item[1].frameBufferSize = sizeof(frameBuffer2);


	ws2812b_init();
}


void visHandle()
{

	if(ws2812b.transferComplete)
	{
		// Update your framebuffer here or swap buffers
		visHandle2();

		// Signal that buffer is changed and transfer new data
		ws2812b.startTransfer = 1;
		ws2812b_handle();
	}
}


