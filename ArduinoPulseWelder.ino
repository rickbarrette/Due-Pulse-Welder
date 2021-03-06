//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ARDUINO PULSE WELDING SYSTEM
// Written by: Zack Goyetche
// Date: 2/21/2015                                                v11.0
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// y = 34.361x + 5.4224
// where y is the current and x is the voltage
// need to find relationship between analog write value and voltage, substitute into x

//Note: Pin 31 - Grey
//      Pin 33 - Purple
//      Pin 35 - Blue
//      Pin 37 - Green

#include "LiquidCrystal.h"
#include "piezoNotes.h"

//DEFINITIONS
#define slope 39.18367346938776
#define slope2 0.0006593406593406593

//INTEGERS
int rs=22,rw=24,en=26,d0=40,d1=42,d2=44,d3=46,d4=28,d5=30,d6=32,d7=34,led=36,oe=38,
	fourTMode=48,melodyPin = 5,ledPin = 52,upSlopePin = A0, downSlopePin = A1, fourTPin = A2;
int asig,bsig,counter,lastCounter,posCounter,slopeTime;
int encoderPin1 = 2;
int encoderPin2 = 3;
int encoderSwitchPin = 4; //push button switch
int outputPin = DAC0;
int song = 0;

float peak,bkgnd,freq,duty,outHigh,outLow, freqPointHigh, freqPointLow;

//BOOLEANS
boolean pushbuttonToggle, ccToggle, ccwToggle, menuSelectToggle, inMenu, incToggle, decToggle, upSlopeToggle,
		downSlopeToggle, downSlopeSet, stepOneToggle, stepTwoToggle, stepThreeToggle, stepFourToggle, twoT_oneToggle,
		twoT_twoToggle;

//LCD DECLARATIONS (rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7)
LiquidCrystal lcd(rs,rw,en,d0,d1,d2,d3,d4,d5,d6,d7);

//SETUP-----------------------------------------------------------------
void setup()
{
	Serial.begin (9600);
	
	//PIN DECLARATIONS
	pinMode(5, OUTPUT); //buzzer
	pinMode(rs, OUTPUT);
	pinMode(rw, OUTPUT);
	pinMode(en, OUTPUT);
	pinMode(d4, OUTPUT);
	pinMode(d5, OUTPUT);
	pinMode(d6, OUTPUT);
	pinMode(d7, OUTPUT);
	pinMode(led, OUTPUT);
	pinMode(ledPin, OUTPUT);
	pinMode(oe, OUTPUT);	
	pinMode(encoderPin1, INPUT);
	pinMode(encoderPin2, INPUT);
	pinMode(encoderSwitchPin, INPUT);
	pinMode(fourTMode, INPUT);
	pinMode(upSlopePin, INPUT);
	pinMode(downSlopePin, INPUT);
	pinMode(fourTPin, INPUT);
	
	//PIN STATES
	digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
	digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
	digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on
	digitalWrite(ledPin, HIGH);
	analogWriteResolution(12);
	
	//INITIAL VARIABLE DECLARATIONS
	peak = 125.00;
	bkgnd = 33.00;
	freq = 33.0;
	duty = 33.0;
	// ^^RULE OF 33's^^
	posCounter = 0;
	outHigh = ((125.0/slope)-0.6)/slope2; //peak current set analogWrite value from 0-4095
	outLow = ((33.0/slope)-0.6)/slope2; //bkgnd current set analogWrite value from 0-4095
	freqPointHigh = int(1000000.0/freq*(duty/100.0)); //initilization of time spent high
	freqPointLow = int((1000000.0/freq)*((100.0-duty)/100.0)); //initialization of time spent low
	
	//ADJUSTABLE PARAMETERS
	slopeTime = 3; //this sets speed of upslope/downslope
	
	//INITIAL BOOLEAN DECLARATIONS
	pushbuttonToggle = false;
	ccToggle = false;
	ccwToggle = false;
	menuSelectToggle = false;
	inMenu = false;
	incToggle = false;
	decToggle = false;
	downSlopeSet = false;
	stepOneToggle = true;
	stepTwoToggle = false;
	stepThreeToggle = false;
	stepFourToggle = false;
	twoT_oneToggle = true;
	twoT_twoToggle = false;
	
	//INTERRUPTS
	attachInterrupt(encoderPin1, A_RISE, RISING);
	attachInterrupt(encoderPin2, B_RISE, RISING);
	
	//OUTPUT ENABLE SEQUENCE FOR LOGIC CONVERTERS
	digitalWrite(oe,LOW);
	delayMicroseconds(100);
	digitalWrite(oe,HIGH);
	delayMicroseconds(100);
	
	//WELCOME SCREEN
	lcd.begin(4,20);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("    Arduino DUE");
	lcd.setCursor(0,1);
	lcd.print(" TIG Pulse Welding  ");
	lcd.setCursor(0,2);
	lcd.print("       System");
	lcd.setCursor(0,3);
	lcd.print("        v1.0");
	sing();
	delayMicroseconds(100);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(">");
	digitalWrite(ledPin, LOW);
}

//MAIN LOOP-------------------------------------------------------------
void loop()
{
	if(digitalRead(fourTMode) == HIGH)
		fourT();
	else
		twoT();
	mainMenu();
	buttonScan();
	encoderScan();
	menuSelect();
}

//INPUT EVENT FUNCTIONS (ROTARY ENCODER)--------------------------------
//----------------------------------------------------------------------
void A_RISE()
{
	detachInterrupt(encoderPin1);
	asig = 1;
	if(bsig == 0)
		counter++;
	if(bsig == 1)
		counter--;
	attachInterrupt(encoderPin1, A_FALL, FALLING);
}

void A_FALL()
{
	detachInterrupt(encoderPin1);
	asig = 0;
	if(bsig == 0)
		counter--;
	if(bsig == 1)
		counter++;
	attachInterrupt(encoderPin1, A_RISE, RISING);
}

void B_RISE()
{
	detachInterrupt(encoderPin2);
	bsig = 1;
	if(asig == 0)
		counter--;
	if(asig == 1)
		counter++;
	attachInterrupt(encoderPin2, B_FALL, FALLING);
}

void B_FALL()
{
	detachInterrupt(encoderPin2);
	bsig = 0;
	if(asig == 0)
		counter++;
	if(asig == 1)
		counter--;
	attachInterrupt(encoderPin2, B_RISE, RISING);
}

void buttonScan()
{
	if(digitalRead(encoderSwitchPin) == LOW)
	{
		pushbuttonToggle = true;
	}
	if(digitalRead(encoderSwitchPin) == HIGH && pushbuttonToggle == true)
	{
		buzz(melodyPin, 4186/10, 40);
		menuSelectToggle = true;
		pushbuttonToggle = false;
	}
	if(digitalRead(upSlopePin == HIGH))
	{
		upSlopeToggle = true;
	}
	else
		upSlopeToggle = false;
	if(digitalRead(downSlopePin == HIGH))
	{
		downSlopeToggle = true;
	}
	else
		downSlopeToggle = false;
	
}

void encoderScan()
{
	if(counter >= lastCounter + 4)
	{
		buzz(melodyPin, 4186/40, 40);
		ccToggle = true;
		incToggle = true;
		lastCounter = counter;
	}
	
	if(counter <= lastCounter -4)
	{
		buzz(melodyPin, 4186/40, 40);
		ccwToggle = true;
		decToggle = true;
		lastCounter = counter;
	}
	if(ccToggle == true)
	{
		ccToggle = false;
		posCounter = posCounter + 1;
		if(posCounter == 4)
		{
			posCounter = 0;
			lcd.setCursor(0,3);
			lcd.print(" ");
			Cursor(posCounter);
		}
		else
		{
			lcd.setCursor(0,posCounter-1);
			lcd.print(" ");
			Cursor(posCounter);
		}
	}
	if(ccwToggle == true)
	{
		ccwToggle = false;
		posCounter = posCounter - 1;
		if(posCounter == -1)
		{
			posCounter = 3;
			lcd.setCursor(0,0);
			lcd.print(" ");
			Cursor(posCounter);
		}
		else
		{
			lcd.setCursor(0,posCounter + 1);
			lcd.print(" ");
			Cursor(posCounter);
		}
	}
}

//MENU FUNCTIONS--------------------------------------------------------
//----------------------------------------------------------------------
void mainMenu()
{
	//PEAK---------------------------
	lcd.setCursor(1,0);
	lcd.print("PEAK: ");
	if(peak < 10)
		lcd.setCursor(8,0);
	else
		lcd.setCursor(7,0);
	lcd.print(int(peak));
	lcd.setCursor(13,0);
	lcd.print("Amps");
	
	//BACKGROUND---------------------------
	lcd.setCursor(1,1);
	lcd.print("BACK: ");
	if(bkgnd < 10)
		lcd.setCursor(8,1);
	else
		lcd.setCursor(7,1);
	lcd.print(int(bkgnd));
	lcd.setCursor(13,1);
	lcd.print("Amps");
	
	//FREQUENCY---------------------------
	lcd.setCursor(1,2);
	lcd.print("FREQ: ");
	if(freq < 10)
		lcd.setCursor(8,2);
	else
		lcd.setCursor(7,2);
	lcd.print(int(freq));
	lcd.setCursor(13,2);
	lcd.print("PPS");
	
	//DUTY CYCLE---------------------------
	lcd.setCursor(1,3);
	lcd.print("DUTY: ");
	if(duty < 10)
		lcd.setCursor(8,3);
	else
		lcd.setCursor(7,3);
	lcd.print(int(duty));
	lcd.setCursor(13,3);
	lcd.print("%");
}

void menuSelect()
{
	if(menuSelectToggle == true && posCounter == 0)
	{
		menuSelectToggle = false;
		inMenu = true;
		incToggle = false; decToggle = false;
		peakMenu();
	}
	
	if(menuSelectToggle == true && posCounter == 1)
	{
		menuSelectToggle = false;
		inMenu = true;
		incToggle = false; decToggle = false;
		bkgndMenu();
	}
	
	if(menuSelectToggle == true && posCounter == 2)
	{
		//posCounter = 0;
		menuSelectToggle = false;
		inMenu = true;
		incToggle = false; decToggle = false;
		freqMenu();
	}
	
	if(menuSelectToggle == true && posCounter == 3)
	{
		menuSelectToggle = false;
		inMenu = true;
		incToggle = false; decToggle = false;
		dutyMenu();
	}
}

void peakMenu()
{
	lcd.clear();
	while(inMenu == true)
	{
		lcd.setCursor(0,0);
		lcd.print("SET PEAK CURRENT");
		lcd.setCursor(0,2);
		lcd.print("PEAK: ");
		lcd.setCursor(6,2);
		lcd.print(int(peak));
		if(peak<100)
		{
			lcd.setCursor(8,2);
			lcd.print(" ");
		}		
		lcd.setCursor(10,2);
		lcd.print("Amps");
		encoderScan();
		buttonScan();
		if(decToggle == true && peak >= 65)
		{
			peak = peak - 5;
			decToggle = false;
		}
		if(incToggle == true && peak <= 155)
		{
			peak = peak + 5;
			incToggle = false;
		}
		
		if(menuSelectToggle == true)
		{
			inMenu = false;
			outHigh = ((peak/slope)-0.6)/slope2;
			menuSelectToggle = false;
			lcd.clear(); lcd.setCursor(0,0); lcd.print(">"); posCounter=0;
		}
	}
}

void bkgndMenu()
{
	lcd.clear();
	while(inMenu == true)
	{
		lcd.setCursor(0,0);
		lcd.print("SET BACKGRND CURRENT");
		lcd.setCursor(0,2);
		lcd.print("BACK: ");
		if(bkgnd < 100)
		{
			lcd.setCursor(6,2);
			lcd.print(int(bkgnd));
			lcd.setCursor(8,2);
			lcd.print("  ");
		}
		else
			lcd.print(int(bkgnd));
		lcd.setCursor(10,2);
		lcd.print("Amps");
		encoderScan();
		buttonScan();
		
		if(decToggle == true && bkgnd == 20)
		{
			bkgnd = bkgnd;
			decToggle = false;
		}
		
		if(decToggle == true && bkgnd == 25)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 30)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 33)
		{
			bkgnd = bkgnd - 3;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 35)
		{
			bkgnd = bkgnd - 2;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 40)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 45)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 50)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 55)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 60)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 65)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 70)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 75)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 80)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 85)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 90)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 95)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 100)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 105)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 110)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 115)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		if(decToggle == true && bkgnd == 120)
		{
			bkgnd = bkgnd - 5;
			decToggle = false;
		}
		
		
		
		
		if(incToggle == true && bkgnd == 120)
		{
			bkgnd = bkgnd;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 115)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 110)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 105)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 100)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 95)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 90)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 85)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 80)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 75)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 70)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 65)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 60)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 55)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 50)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 45)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 40)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 35)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 33)
		{
			bkgnd = bkgnd + 2;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 30)
		{
			bkgnd = bkgnd + 3;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 25)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		if(incToggle == true && bkgnd == 20)
		{
			bkgnd = bkgnd + 5;
			incToggle = false;
		}
		
		if(menuSelectToggle == true)
		{
			inMenu = false;
			outLow = ((bkgnd/slope)-0.6)/slope2;
			menuSelectToggle = false;		
			lcd.clear(); lcd.setCursor(0,0); lcd.print(">"); posCounter=0;
		}
	}
}

void freqMenu()
{
	lcd.clear();
	while(inMenu == true)
	{
		lcd.setCursor(0,0);
		lcd.print("SET PULSE FREQUENCY");
		lcd.setCursor(0,2);
		lcd.print("FREQ: ");
		lcd.setCursor(6,2);
		lcd.print(int(freq));
		if(freq<10)
		{
			lcd.setCursor(7,2);
			lcd.print(" ");
		}
		if(freq<100)
		{
			lcd.setCursor(8,2);
			lcd.print(" ");
		}
		if(freq<1000)
		{
			lcd.setCursor(9,2);
			lcd.print(" ");
		}
		lcd.setCursor(11,2);
		lcd.print("PPS");
		encoderScan();
		buttonScan();
		if(decToggle == true && freq > 1 && freq <= 10)
		{
			freq = freq - 1;
			decToggle = false;
		}
		if(incToggle == true && freq >= 1 && freq < 10)
		{
			freq = freq + 1;
			incToggle = false;
		}
		if(incToggle == true && freq >= 10 && freq < 30)
		{
			freq = freq + 10;
			incToggle = false;
		}	
		if(incToggle == true && freq == 30)
		{
			freq = freq + 3;
			incToggle = false;
		}	
		if(incToggle == true && freq == 33)
		{
			freq = freq + 7;
			incToggle = false;
		}	
		if(incToggle == true && freq >= 40 && freq < 100)
		{
			freq = freq + 10;
			incToggle = false;
		}
		if(decToggle == true && freq == 40)
		{
			freq = freq - 7;
			decToggle = false;
		}
		if(decToggle == true && freq == 33)
		{
			freq = freq - 3;
			decToggle = false;
		}
		if(decToggle == true && freq > 10 && freq <= 30)
		{
			freq = freq - 10;
			decToggle = false;
		}
		if(decToggle == true && freq > 40 && freq <= 100)
		{
			freq = freq - 10;
			decToggle = false;
		}
		if(incToggle == true && freq >= 100 && freq < 1000)
		{
			freq = freq + 100;
			incToggle = false;
		}
		if(decToggle == true && freq > 100 && freq <= 1000)
		{
			freq = freq - 100;
			decToggle = false;
		}
		if(menuSelectToggle == true)
		{
			inMenu = false;
			// freqPointHigh/Low sets the proper delays for corresponding freq and duty settings
			freqPointHigh = int(1000000.0/freq*(duty/100.0));
			// (1000000 us/Hz) * (%time spend high)
			freqPointLow = int((1000000.0/freq)*((100.0-duty)/100.0));
			// (1000000 us/Hz) * (100%-%time spend high)
			menuSelectToggle = false;
			lcd.clear(); lcd.setCursor(0,0); lcd.print(">"); posCounter=0;
		}
	}
}

void dutyMenu()
{
	lcd.clear();
	while(inMenu == true)
	{
		lcd.setCursor(0,0);
		lcd.print("SET DUTY CYCLE");
		lcd.setCursor(0,2);
		lcd.print("DUTY: ");
		lcd.setCursor(6,2);
		lcd.print(int(duty));
		lcd.setCursor(10,2);
		lcd.print("%");
		
		encoderScan();
		buttonScan();
		if(decToggle == true && duty == 25)
		{
			duty = duty;
			decToggle = false;
		}
		if(decToggle == true && duty == 30)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 33)
		{
			duty = duty - 3;
			decToggle = false;
		}
		if(decToggle == true && duty == 35)
		{
			duty = duty - 2;
			decToggle = false;
		}
		if(decToggle == true && duty == 40)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 45)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 50)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 55)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 60)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 65)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 70)
		{
			duty = duty - 5;
			decToggle = false;
		}
		if(decToggle == true && duty == 75)
		{
			duty = duty - 5;
			decToggle = false;
		}
		
		
		if(incToggle == true && duty == 75)
		{
			duty = duty;
			incToggle = false;
		}
		if(incToggle == true && duty == 70)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 65)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 60)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 55)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 50)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 45)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 40)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 35)
		{
			duty = duty + 5;
			incToggle = false;
		}
		if(incToggle == true && duty == 33)
		{
			duty = duty + 2;
			incToggle = false;
		}
		if(incToggle == true && duty == 30)
		{
			duty = duty + 3;
			incToggle = false;
		}
		if(incToggle == true && duty == 25)
		{
			duty = duty + 5;
			incToggle = false;
		}
		
		
		if(menuSelectToggle == true)
		{
			inMenu = false;
			// freqPointHigh/Low sets the proper delays for corresponding freq and duty settings
			freqPointHigh = int(1000000.0/freq*(duty/100.0)); 
			// (1000000 us/Hz) * (%time spend high)
			freqPointLow = int((1000000.0/freq)*((100.0-duty)/100.0)); 
			// (1000000 us/Hz) * (100%-%time spend high)
			menuSelectToggle = false;
			lcd.clear(); lcd.setCursor(0,0); lcd.print(">"); posCounter=0;	
		}
	}
}

void Cursor(int i)
{
	if(inMenu == false)
	{
		lcd.setCursor(0,i);
		lcd.print(">");
	}
}

//OUTPUT FUNCTIONS------------------------------------------------------
//----------------------------------------------------------------------
void pulseOut()
{
	//0.6V = 0, 3.3V = 4095
	//current = v*slope
	//analog value -> voltage -> current
	//peak/slope/slope2
	//line eqn. for 3.3V ... (0,0.6) , (4095,3.3)
	//(y2-y1)/(x2-x1) ===> (3.3-0.6)/(4095-0) ===> 2.7/4095 = 0.0006593406593406593
	
	//if upslope, step up output current at a constant slope proportional to both
	//min/max current and the frequency of pulsing.
	
	analogWrite(outputPin,outHigh);
	delayMicroseconds(freqPointHigh);
	analogWrite(outputPin,outLow);
	delayMicroseconds(freqPointLow);
}

void upSlope()
{
	float differential = outHigh - outLow; //this value is the difference between peak and bkgnd current settings
	float upSlopeWrite; //declare this value for upSlope analog writes
	
	// in order to upslope proportional to the frequency, there must be a scaling factor that
	// that correlates with the number of intermediate values and the set frequency.
	// example ... if i want the upSlope to always take 10 seconds, then there should be
	// n intermediate values per second.
	// n = 10*freq --> freq of 10 per second, there will be 100 intermediate steps, each taking 1/10th of a second
	// 1/10th of a second * 100 = 10 seconds.
	int n;
	int j;
	n = slopeTime*freq;
	float segment = differential/n;
	upSlopeWrite = outLow;
	
	for(j=n;j>0;j--)
	{
		//Serial.println(upSlopeWrite);
		analogWrite(outputPin,upSlopeWrite);
		delayMicroseconds(freqPointHigh);
		analogWrite(outputPin,outLow);
		delayMicroseconds(freqPointLow);
		upSlopeWrite = upSlopeWrite + segment;
	}
	//pulseOut();
}

void downSlope()
{
	float differential = outHigh - outLow; //this value is the difference between peak and bkgnd current settings
	float downSlopeWrite; //declare this value for upSlope analog writes
	
	// in order to upslope proportional to the frequency, there must be a scaling factor that
	// that correlates with the number of intermediate values and the set frequency.
	// example ... if i want the upSlope to always take 10 seconds, then there should be
	// n intermediate values per second.
	// n = 10*freq --> freq of 10 per second, there will be 100 intermediate steps, each taking 1/10th of a second
	// 1/10th of a second * 100 = 10 seconds.
	
	int n;
	int j;
	n = slopeTime*freq;
	float segment = differential/n;
	downSlopeWrite = outHigh;
	
	for(j=n;j>0;j--)
	{
		//Serial.println(upSlopeWrite);
		analogWrite(outputPin,downSlopeWrite);
		delayMicroseconds(freqPointHigh);
		analogWrite(outputPin,outLow);
		delayMicroseconds(freqPointLow);
		downSlopeWrite = downSlopeWrite - segment;
	}
	analogWrite(outputPin,0); //shuts off (this is 2T mode with downSlope)
}

void fourT()
{
	//*******STEP 1*******
	if(stepOneToggle == true && digitalRead(fourTPin) == HIGH)
	{
		digitalWrite(ledPin, HIGH);
		stepOneToggle = false;
		stepTwoToggle = true;
		while(digitalRead(fourTPin) == HIGH)
		{
			analogWrite(outputPin,outLow);
		}
	}
	
	//*******STEP 2*******
	if(stepTwoToggle == true && digitalRead(fourTPin) == LOW)
	{
		stepTwoToggle = false;
		stepThreeToggle = true;
		upSlope();
		while(digitalRead(fourTPin) == LOW)
		{
			pulseOut();
		}
	}
	
	//*******STEP 3*******
	if(stepThreeToggle == true && digitalRead(fourTPin) == HIGH)
	{
		stepThreeToggle = false;
		stepFourToggle = true;
		downSlope();
		while(digitalRead(fourTPin) == HIGH)
		{
			analogWrite(outputPin,outLow);
		}
	}
	
	//*******STEP 4*******
	if(stepFourToggle == true && digitalRead(fourTPin) == LOW)
	{
		stepFourToggle = false;
		stepOneToggle = true;
		analogWrite(outputPin,0);
		digitalWrite(ledPin,LOW);	
	}	
}

void twoT()
{
	if(digitalRead(fourTPin) == HIGH && twoT_oneToggle == true) //upSlope enabled condition
	{
		twoT_oneToggle = false;
		twoT_twoToggle = true;
		digitalWrite(ledPin, HIGH);
		if(digitalRead(downSlopePin) == HIGH) //check to see if downSlope is enabled
			downSlopeSet = true; //set boolean to initiate downSlope on shut off
		if(digitalRead(upSlopePin) == HIGH)
			upSlope(); //call upSlope function
		while(digitalRead(fourTPin) == HIGH)
		{
			
		}
		while(digitalRead(fourTPin) == LOW)
			pulseOut(); //begin pulsing
	}
	if(digitalRead(fourTPin) == HIGH && twoT_twoToggle == true) //downslope not working! led not turning off!
	{
		twoT_twoToggle = false;
		twoT_oneToggle = true;
		if(downSlopeSet == true)
		{
			downSlopeSet = false;
			downSlope();
		}
		analogWrite(outputPin, 0);
		digitalWrite(ledPin, LOW);
		while(digitalRead(fourTPin) == HIGH)
		{
			
		}
	}
}

void footPedal()
{
	//2T and 4T mode
	
	//2T... push in - ON, release push - OFF.
	//upSlope and downSlope will occur if enabled
	
	//4T... push in - *background current initiated*, release push - upslope begins to peak
	//      push in - downslope begins to background current, release push - *arc shut off*
	
	//SUMMARY:
	// 1) arc on to background current setting (first push)
	// 2) upslope begins to peak               (first release)
	// 3) downslope back to background         (second push)
	// 4) arc off                              (second release)
}

//PIEZO FUNCTIONS-------------------------------------------------------
//----------------------------------------------------------------------
int intro[] =
{
	NOTE_E7, NOTE_E7, 0, NOTE_E7,
	0, NOTE_C7, NOTE_E7, 0,
	NOTE_G7, 0, 0,  0,
	NOTE_G6, 0, 0, 0
}; //Mario intro melody

int introTempo[] =
{
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12,
	12, 12, 12, 12
};

void sing()
{
	// iterate over the notes of the melody:
	int size = sizeof(intro) / sizeof(int);
	for (int thisNote = 0; thisNote < size; thisNote++)
	{
		// to calculate the note duration, take one second
		// divided by the note type.
		//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
		int noteDuration = 1000 / introTempo[thisNote];
			
		buzz(melodyPin, intro[thisNote], noteDuration);
			
		// to distinguish the notes, set a minimum time between them.
		// the note's duration + 30% seems to work well:
		int pauseBetweenNotes = noteDuration * 1.30;
		delay(pauseBetweenNotes);
			
		// stop the tone playing:
		buzz(melodyPin, 0, noteDuration);
	}
}

void buzz(int targetPin, long frequency, long length)
{
	long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
	//// 1 second's worth of microseconds, divided by the frequency, then split in half since
	//// there are two phases to each cycle
	long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
	//// multiply frequency, which is really cycles per second, by the number of seconds to
	//// get the total number of cycles to produce
	for (long i = 0; i < numCycles; i++)
	{ // for the calculated length of time...
		digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
		delayMicroseconds(delayValue); // wait for the calculated delay value
		digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
		delayMicroseconds(delayValue); // wait again or the calculated delay value
	}
}