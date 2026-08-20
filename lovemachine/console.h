#pragma once
#include "includes.h"
#include "definitions.h"
#include "color.h"

//#define DEBUG_LOG

ofstream myfile;

// TODO: ñäåëàòü ýòî äåðüìî þçàáåëüíûì
namespace console
{
	// òèï êîíñîëè
	enum console_type : int
	{
		window,
		con_menu
	};
	int type = window;

	// öâåòà // TODO: äîáàâèòü àäàïòàöèþ öâåòîâ ïîä ingame êîíñîëü
	#define darkgreen 2
	#define darkred 12
	#define darkwhite 7
	#define green 10
	#define red 4
	const int defcolor = 11;

	struct console_line
	{
		console_line(string text, color ccolor)
		{
			this->text = text;
			this->ccolor = ccolor;
		}
		
		string text;
		color ccolor;
	};

	string cur_text = str("no text");
	color cur_color;
	deque<console_line> lines;

	handle hconsole, herror;

	void set_color(int color = -1)
	{
		if (color == -1)
		{
			SetConsoleTextAttribute(hconsole, defcolor);
		}
		else
		{
			SetConsoleTextAttribute(hconsole, color);
		}
	}

	void write(string text, int color = -1)
	{
		set_color(color);
		cout << text << endl;
		myfile << text << endl;
	}

	void write_hex(string text, DWORD val, int color = -1)
	{
		set_color(color);
		cout << text << " : " << "0x" << val << endl;
		myfile << text << " : " << "0x" << val << endl;
		cout << dec;
		myfile << dec;
	}

	void add_line()
	{
		lines.push_back(console_line(cur_text, cur_color));
	}

	void write(string text, color ccolor, int ctype = con_menu)
	{
		cur_text = text;
		cur_color = ccolor;
		add_line();
	}

	void create(string name)
	{
		if (type == window)
		{
			AllocConsole();
			FILE* fp = nullptr;
			freopen_s(&fp, "CONIN$", "r", stdin);
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);

			SetConsoleTitleA(name.c_str()); // óñòàíîâêà èìåíè

			hconsole = GetStdHandle(STD_OUTPUT_HANDLE); // õåíäë ïîòîêà âûâîäà
			herror = GetStdHandle(STD_ERROR_HANDLE); // õåíäë ïîòîêà îøèáîê

			SetConsoleTextAttribute(hconsole, defcolor);
			SetConsoleTextAttribute(herror, red);

			write("lovemachine injected\nconsole created");
		}
	}

	void remove()
	{
		if (hconsole == NULL || herror == NULL) return;

		console::write("removing console");
		FreeConsole();
	}
}