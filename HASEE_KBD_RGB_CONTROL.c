#include <stdio.h>

#include <Windows.h>

#pragma comment(lib, "Winmm.lib")

extern long long (*g_setdchu_data_fn)(int, void *, int);
extern char g_flow_light_enabled;
extern char g_breathe_light_enabled;
extern int g_delay;

unsigned thread_rgb_ctrl(void *in_vars)
{
	//char /*tran_delay*/ = argc > 1 ? atoi(argv[1]) : 5;

	long long ret_ll = 0;

	unsigned int led_off_data_0 = 0xF4000000;
	unsigned int led_off_data_1 = 0xE0000007;
	unsigned int led_on_data_0 = 0xF40000FF;
	unsigned int led_on_data_1 = 0xE007B001;

	unsigned int kbd_led_color_data_ori = 0xF0FFFFFF;
	unsigned int kbd_led_color_data_curr = 0xF0FFFFFF;

	timeBeginPeriod(1);

	// ret_ll = setdchu_data(103, &led_on_data_0, 4);
	// ret_ll = setdchu_data(103, &led_on_data_1, 4);

	// for (unsigned char i = 0; i < 255; i++) {

	// }

	char curr_tran_stat = 0;
	unsigned char r_tran = 0xFF;
	unsigned char g_tran = 0;
	unsigned char b_tran = 0;

	unsigned int color = 0;

	char add = 1;
	unsigned char count = 0;
	for (;;) {
		// flowing light
		if (g_flow_light_enabled) {
			switch (curr_tran_stat) {
				case 0:
					r_tran -= 5;
					g_tran += 5;
					if (g_tran == 255) {
						curr_tran_stat = 1;
					}
					break;

				case 1:
					g_tran -= 5;
					b_tran += 5;
					if (b_tran == 255) {
						curr_tran_stat = 2;
					}
					break;

				case 2:
					b_tran -= 5;
					r_tran += 5;
					if (r_tran == 255) {
						curr_tran_stat = 0;
					}
					break;
			}

			kbd_led_color_data_curr &= 0xFF000000;
			kbd_led_color_data_curr |= (((int)b_tran << 16) | (int)r_tran << 8 | g_tran);

			ret_ll = g_setdchu_data_fn(103, &kbd_led_color_data_curr, 4);
		}

		// breathe light
		if (g_breathe_light_enabled) {
			led_on_data_0 &= 0xFFFFFF00;
			led_on_data_0 |= count;

			ret_ll = g_setdchu_data_fn(103, &led_on_data_0, 4);
			ret_ll = g_setdchu_data_fn(103, &led_on_data_1, 4);

			if (!count) {
				add = 1;
			}

			if (count == 255) {
				add = 0;
			}

			add ? count++ : count--;
		}

		Sleep(g_delay);
	}

	// unsigned int time_delay = 5;
	// unsigned char count = 0;
	// for (;;) {
	// 	led_on_data_0 &= 0xFFFFFF00;
	// 	led_on_data_0 |= count;

	// 	ret_ll = setdchu_data(103, &led_on_data_0, 4);
	// 	ret_ll = setdchu_data(103, &led_on_data_1, 4);

	// 	if (!count) {
	// 		add = 1;
	// 	}

	// 	if (count == 255) {
	// 		add = 0;
	// 	}

	// 	add ? count++ : count--;

	// 	Sleep(time_delay);
	// }

	return 0;
}
