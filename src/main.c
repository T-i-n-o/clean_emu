#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "AsyncIO.h"
#include "TUM_FreeRTOS_Utils.h"
#include "TUM_Draw.h"
#include "TUM_Utils.h"
#include "TUM_Event.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

TaskHandle_t TestScreen = NULL;
TaskHandle_t BufferSwap = NULL;

SemaphoreHandle_t DrawSignal;

void vTestScreen() {

    while(1) {

        if(DrawSignal) {
            if(xSemaphoreTake(DrawSignal, portMAX_DELAY) == pdTRUE) {
                
                tumDrawClear(Silver);
                
                vTaskDelay((TickType_t) 1000);
            }
        }
    }
}

void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 20;

    while (1) {
        tumDrawUpdateScreen();
        tumEventFetchEvents(FETCH_EVENT_BLOCK);
        xSemaphoreGive(DrawSignal);
        vTaskDelayUntil(&xLastWakeTime,
                        pdMS_TO_TICKS(frameratePeriod));
    }
}

int main(int argc, char *argv[])
{
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);

	printf("Initializing: ");

	if (tumDrawInit(bin_folder_path)) {
		printf("Failed to initialize drawing");
        return EXIT_FAILURE;
	}

    if (tumEventInit()) {
		printf("Failed to initialize Events");
        return EXIT_FAILURE;
	}

    DrawSignal = xSemaphoreCreateBinary(); // Screen buffer locking
    if (!DrawSignal) {
        printf("Failed to create draw signal");
        return EXIT_FAILURE;
    }

	xTaskCreate(vTestScreen, "TestScreen", 
            mainGENERIC_STACK_SIZE , NULL,
			mainGENERIC_PRIORITY, &TestScreen);
    
    xTaskCreate(vSwapBuffers, "BufferSwap", 
            mainGENERIC_STACK_SIZE , NULL,
			mainGENERIC_PRIORITY, &BufferSwap); 

    tumFUtilPrintTaskStateList();

	vTaskStartScheduler();

	return EXIT_SUCCESS;
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
	/* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
	/* Makes the process more agreeable when using the Posix simulator. */
	xTimeToSleep.tv_sec = 1;
	xTimeToSleep.tv_nsec = 0;
	nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
