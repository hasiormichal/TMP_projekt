/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					
					Temat:
					Symulacja prostej centrali alarmowej
					
					autorzy:
					Michal Hasior
					Andrzej Filipowski
----------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "ADC.h"
//#include "pit.c"
#include "pit.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

float volt_coeff = ((float)(((float)2.91) / 4095) );			// Wspólczynnik korekcji wyniku, w stosunku do napiecia referencyjnego przetwornika
uint8_t wynik_ok=0;
uint16_t temp;
float	wynik;
float bufor;
int pom=0;
void PIT_IRQHandler()
{
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;		// Skasuj flage zadania przerwania
	//PIT->CHANNEL[0].TCTRL = 0;									// zatrzymaj licznik PIT0
	NVIC_ClearPendingIRQ(PIT_IRQn);
}

void ADC0_IRQHandler()
{	
	
	temp = ADC0->R[0];		// Odczyt danej i skasowanie flagi COCO
	if(!wynik_ok)					// Sprawdz, czy wynik skonsumowany przez petle glówna
	{
		pom++;
		wynik =(temp + wynik);					// Wyslij nowa dana do petli glównej
		if(pom ==10){ //dlaczego taka duza wartosc
			wynik = wynik/10;
			wynik_ok=1;
			pom=0;
		}
	}
	NVIC_EnableIRQ(ADC0_IRQn);
}
int main (void)
{
	uint8_t	kal_error;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	LCD1602_Init();		 // Inicjalizacja wyswietlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_Print("---");		// Ekran kontrolny - nie zniknie, jesli dalsza czesc programu nie dziala
	PIT_Init();							// Inicjalizacja licznika PIT0
	kal_error=ADC_Init();		// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{
		while(1);							// Klaibracja sie nie powiodla
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(0);		// Odblokowanie przerwania i wybranie kanalu nr 0
	
	while(1)
	{
		if(wynik_ok)
		{
			wynik = wynik*volt_coeff;		// Dostosowanie wyniku do zakresu napieciowego
			sprintf(display,"U=%.4fV",wynik);
			LCD1602_SetCursor(0,0);
			LCD1602_Print(display);
			wynik_ok=0;
		}
	}
}
