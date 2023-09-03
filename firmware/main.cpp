#include <Arduino.h>
#include <U8g2lib.h>

constexpr int BUTTON_PIN = 0;
constexpr int ENCODER_PIN_A = 5;
constexpr int ENCODER_PIN_B = 4;

//#define DISPLAY_ENABLED
constexpr int DISPLAY_CLK = 14;
constexpr int DISPLAY_DATA = 13;
constexpr int DISPLAY_CS = 15;
constexpr int DISPLAY_RESET = 16;
constexpr int DISPLAY_DC = 2;

constexpr int BTN_DEBOUNCE_MS = 50;
constexpr int BTN_CLICK_THRESH = 300;
constexpr int BTN_DOUBLECLICK_THRESH = 150;

#ifdef DISPLAY_ENABLED
U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, DISPLAY_CS, DISPLAY_DC, DISPLAY_RESET);
#endif

int enc_old_a;
int btn_old;
unsigned long btn_time;
unsigned long btn_debounce;

int display_millis = 0;
int enc_pos = 0;
bool btn_pressed = false;

enum BTN_STATE
{
	UP = 0,
	DOWN = 1,
	DOWN_PRE = 2,
	DOWN_HOLD = 3,
	CLICK = 4,
	DOWN_DOUBLE = 5,
	DOWN_SCROLL = 6,
	DOWN_DOUBLESCROLL = 7
};

enum ROTARY_STATE
{
	LEFT = -1,
	IDLE = 0,
	RIGHT = 1,
};

ROTARY_STATE rotary = ROTARY_STATE::IDLE;
BTN_STATE btn = BTN_STATE::UP;

void setup()
{
	pinMode(BUTTON_PIN, INPUT);
	pinMode(ENCODER_PIN_A, INPUT_PULLUP);
	pinMode(ENCODER_PIN_B, INPUT_PULLUP);

	Serial.begin(9600);
	
	#ifdef DISPLAY_ENABLED
	u8g2.begin();
	#endif

	enc_old_a = digitalRead(ENCODER_PIN_A);
	btn_old = digitalRead(BUTTON_PIN);

	btn_time = millis();
}

void printRotation(int btn_state=-1)
{
	BTN_STATE state = btn;

	if (btn_state != -1)
	{
		state = static_cast<BTN_STATE>(btn_state);
	}
	
	switch (state)
	{
		case BTN_STATE::DOWN_DOUBLESCROLL:
			Serial.print("enc1_doubleclick_scroll");
			break;
		case BTN_STATE::DOWN_SCROLL:
			Serial.print("enc1_hold_scroll");
			break;
		case BTN_STATE::UP:
			Serial.print("enc1_scroll");
			break;
		default:
			return;
	}

	switch (rotary)
	{
		case ROTARY_STATE::RIGHT:
			Serial.println(" up");
			enc_pos++;
			break;
		case ROTARY_STATE::LEFT:
			Serial.println(" down");
			enc_pos--;
			break;
	}
}

void printBtn()
{
	switch (btn)
	{
		case BTN_STATE::DOWN_HOLD:
			Serial.println("sw_btn_hold");
			break;
		case BTN_STATE::UP:
			Serial.println("sw_btn_up");
			break;
		case BTN_STATE::CLICK:
			Serial.println("sw_btn_click");
			break;
		case BTN_STATE::DOWN_DOUBLE:
			Serial.println("sw_btn_doubleclick");
	}
}

void draw_display()
{
	#ifdef DISPLAY_ENABLED

	char buf[1];
	u8g2.firstPage();
	do
	{
		u8g2.setFont(u8g2_font_unifont_t_0_76);
		u8g2.drawGlyph(0, 16, 9762);
		u8g2.drawStr(18,16,"headless");
		
		u8g2.drawGlyph(0, 32, 9843 + enc_pos % 8);
		if (btn_pressed)
			u8g2.drawStr(16, 32, "*");

		u8g2.setFont(u8g2_font_roentgen_nbp_tn);
		u8g2.drawStr(0, 64, strcat(itoa(display_millis, buf, 10), "ns"));
	} while (u8g2.nextPage());

	#endif
}

void loop()
{
	int enc_a = digitalRead(ENCODER_PIN_A);
	int enc_b = digitalRead(ENCODER_PIN_B);

	if (enc_a != enc_old_a && enc_a == 1)
	{
		if (enc_b != enc_a)
		{
			rotary = ROTARY_STATE::LEFT;
		}
		else
		{
			rotary = ROTARY_STATE::RIGHT;
		}
	}
	else
	{
		rotary = ROTARY_STATE::IDLE;
	}

	enc_old_a = enc_a;

	int btn_s = digitalRead(BUTTON_PIN);

	if (btn_s == HIGH)
		btn_pressed = true;
	else
		btn_pressed = false;

	if (rotary != ROTARY_STATE::IDLE)
	{
		if (btn_s == LOW)
		{
			switch (btn)
			{
				case BTN_STATE::DOWN_DOUBLE:
					btn = BTN_STATE::DOWN_DOUBLESCROLL;
					break;
				default:
					btn = BTN_STATE::DOWN_SCROLL;
					break;
			}
			printRotation();
		}
		else
		{
			printRotation(BTN_STATE::UP);
		}
	}
	else
	{
		if (btn_s == 0)
			switch(btn)
			{
				case BTN_STATE::UP:
					btn = BTN_STATE::DOWN_PRE;
					btn_time = millis();
					break;
				case BTN_STATE::DOWN_PRE:
					if (millis() - btn_time > BTN_CLICK_THRESH)
					{
						btn = BTN_STATE::DOWN_HOLD;
						btn_time = millis();
						printBtn();
					}
					break;
				case BTN_STATE::CLICK:
					if (millis() - btn_time <= BTN_DOUBLECLICK_THRESH)
					{
						btn = BTN_STATE::DOWN_DOUBLE;
						btn_time = millis();
					}
					break;
			}
		else
		{
			switch(btn)
			{
				case BTN_STATE::DOWN_PRE:
					if (millis() - btn_time <= BTN_CLICK_THRESH)
					{
						btn = BTN_STATE::CLICK;
						btn_time = millis();
					}
					else
					{
						btn = BTN_STATE::UP;
						printBtn();
					}
					break;
				case BTN_STATE::CLICK:
					if (millis() - btn_time > BTN_DOUBLECLICK_THRESH)
					{
						printBtn();
						btn = BTN_STATE::UP;
					}
					break;
				case BTN_STATE::DOWN_HOLD:
					btn = BTN_STATE::UP;
					printBtn();
					break;
				case BTN_STATE::DOWN_DOUBLE:
					printBtn();
					btn = BTN_STATE::UP;
					break;
				default:
					btn = BTN_STATE::UP;
			}
		}
	}

	size_t draw_time = millis();
	draw_display(); // todo subtract from millis
	display_millis = millis() - draw_time;

	delay(1);
}
