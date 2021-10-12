#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <conio.h>
#include <string>
#include <fstream>
#include <Windows.h>
#include <cmath> 
#include <chrono>
#include <ctime>

using namespace std;

const int SCREEN_WIDTH = 880;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

class Character		 //класс персонажа
{
private:
	int CURRENT_HUNGER;		//параметр сытости
	int CURRENT_FUN;		//параметр настроения
	int CURRENT_ENERGY;		//параметр энергии
	int GOLD;				//параметр золота
	int KNOWLEDGE;			//параметр знаний
public:
	Character(int CURRENT_HUNGER0 = 100, int CURRENT_FUN0 = 100, int CURRENT_ENERGY0 = 100, int GOLD0 = 0, int KNOWLEDGE0 = 0) : CURRENT_HUNGER(CURRENT_HUNGER0), CURRENT_FUN(CURRENT_FUN0), CURRENT_ENERGY(CURRENT_ENERGY0), GOLD(GOLD0), KNOWLEDGE(KNOWLEDGE0) {};
	void print() {
		cout << "H:" << CURRENT_HUNGER << "\nF:" << CURRENT_FUN << "\nE:" << CURRENT_ENERGY << "\nG:" << GOLD << "\nK:" << KNOWLEDGE << "\n";
	}
	void scan() {
		cout << "input hunger: ";
		cin >> CURRENT_HUNGER;
		cout << "\ninput fun: ";
		cin >> CURRENT_FUN;
		cout << "\ninput energy: ";
		cin >> CURRENT_ENERGY;
		cout << "\ninput gold";
		cin >> GOLD;
		cout << "\ninput knowledge";
		cin >> KNOWLEDGE;
		cout << "\n";
	}
	void decrease_gold(int gold) {
		GOLD -= gold;
	}
	void decrease_knowledge(int points) {
		KNOWLEDGE -= points;
	}
	void decrease_hunger() {
		CURRENT_HUNGER--;
	}
	void decrease_fun() {
		CURRENT_FUN--;
	}
	void decrease_energy() {
		CURRENT_ENERGY--;
	}
	void increase_gold(int gold) {
		GOLD += gold;
	}
	void increase_knowledge(int points) {
		KNOWLEDGE += points;
	}
	void increase_hunger(int food) {
		CURRENT_HUNGER += food;
	}
	void increase_fun(int smthforfun) {
		CURRENT_FUN += smthforfun;
	}
	void increase_energy(int energy) {
		CURRENT_ENERGY += energy;
	}
	int& set_gold() { return GOLD; }
	int& set_knowledge() { return KNOWLEDGE; }
	int& set_hunger() { return CURRENT_HUNGER; }
	int& set_fun() { return CURRENT_FUN; }
	int& set_energy() { return CURRENT_ENERGY; }
	int get_gold() { return GOLD; }
	int get_knowledge() { return KNOWLEDGE; }
	int get_hunger() { return CURRENT_HUNGER; }
	int get_fun() { return CURRENT_FUN; }
	int get_energy() { return CURRENT_ENERGY; }
};

void HideConsole() {
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}
void ShowConsole() {
	::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}

SDL_Texture* loadTexture(const string& file, SDL_Renderer* ren)
{
	SDL_Texture* texture = IMG_LoadTexture(ren, file.c_str());
	if (!texture)
	{
		cout << IMG_GetError(); // Можно заменить на SDL_GetError()
	}
	return texture;
}

SDL_Texture* renderText(const string& message, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer)
{
	//Сначала нужно отобразить на поверхность с помощью TTF_RenderText,
	//затем загрузить поверхность в текстуру
	SDL_Surface* surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == nullptr) {
		TTF_CloseFont(font);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_RenderText");
		return nullptr;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "CreateTexture");
	}
	SDL_FreeSurface(surf);
	return texture;
}

void renderTexture_m(SDL_Texture* tex, SDL_Renderer* ren, int x, int y, int w, int h)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

void renderTexture_notm(SDL_Texture* tex, SDL_Renderer* ren, int x, int y)
{
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture_m(tex, ren, x, y, w, h);
}

void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, SDL_Rect dst,
	SDL_Rect* clip = nullptr)
{
	SDL_RenderCopy(ren, tex, clip, &dst);
}

void renderTexture_(SDL_Texture* tex, SDL_Renderer* ren, int x, int y,
	SDL_Rect* clip = nullptr)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != nullptr) {
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	renderTexture(tex, ren, dst, clip);
}

void renderCloud(SDL_Texture* tex, SDL_Renderer* ren, int stat, int fix_down, int fix_up, int x, int y, int w, int h)
{
	if (stat < (fix_up + 1) && stat > fix_down)
		renderTexture_m(tex, ren, x, y, w, h);
}

void check_width_for_texture(int param, int& res) {
	if (param > 9)
		res = 30;
	else
		res = 15;
	if (param >= 100)
		res = 45;
}

int number_color(int inp) {		// 1 = black, 0 = red
	if (inp > 20) return 1;
	else return 0;
}

int main(int argc, char* argv[])
{
	HideConsole();
	Character Student;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)		//Инициализируем все подсистемы (есть различные наборы инструментов внутри самой библиотеки, которые по умолчанию не используются). Нам нужна как минимум подсистема VIDEO, но EVERYTHING итак содержит ее в себе, вместе со всеми остальными
	{
		cout << "SDL_Init Error: " << SDL_GetError() << endl;
		return 1;
	}

	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
		//logSDLError(cout, "IMG_Init");
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "img init");
		SDL_Quit();
		return 1;
	}

	if (TTF_Init() != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_Init");
		SDL_Quit();
		return 1;
	}

	window = SDL_CreateWindow("MIEMchik", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);	//Создание окна (название, точка x, точка y, ширина, высота, флаги (доп. параметры окна, которые можно посмотреть здесь http://wiki.libsdl.org/SDL_WindowFlags))

	if (window == nullptr)			//Проверяем: создается ли окно. GetError покажет причину ошибки, если она будет
	{
		cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
		return 1;
		SDL_Quit();
		IMG_Quit();
		TTF_Quit();
	}

	SDL_SetWindowResizable(window, SDL_TRUE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);	//Создаем рендерер, чтобы с внутренней частью окна можно было работать

	if (renderer == nullptr)			//Проверяем, создается ли рендерер
	{
		cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
		SDL_Quit();
		IMG_Quit();
		TTF_Quit();
		return 1;
	}

	SDL_Texture* background = loadTexture("Pics//background.png", renderer);
	SDL_Texture* background_miem = loadTexture("Pics//background_miem.png", renderer);
	SDL_Texture* image = loadTexture("Pics//image.png", renderer);
	SDL_Texture* pizza = loadTexture("Pics//pizza.png", renderer);
	SDL_Texture* smile = loadTexture("Pics//smile.png", renderer);
	SDL_Texture* lightning = loadTexture("Pics//lightning.png", renderer);
	SDL_Texture* gold = loadTexture("Pics//gold.png", renderer);
	SDL_Texture* books = loadTexture("Pics//books.png", renderer);
	SDL_Texture* student_pic_full = loadTexture("Pics//student_pic_full.png", renderer);
	SDL_Texture* student_pic_tired = loadTexture("Pics//student_pic_tired.png", renderer);
	SDL_Texture* button_purple_hunger = loadTexture("Pics//button_purple_hunger.png", renderer);
	SDL_Texture* button_purple_fun = loadTexture("Pics//button_purple_fun.png", renderer);
	SDL_Texture* button_purple_energy = loadTexture("Pics//button_purple_energy.png", renderer);
	SDL_Texture* button_purple_play = loadTexture("Pics//button_purple_play.png", renderer);
	SDL_Texture* button_purple_quit = loadTexture("Pics//button_purple_quit.png", renderer);
	SDL_Texture* button_purple_menu = loadTexture("Pics//button_purple_menu.png", renderer);
	SDL_Texture* button_purple_save = loadTexture("Pics//button_purple_save.png", renderer);
	SDL_Texture* end_screen = loadTexture("Pics//end_screen.png", renderer);
	SDL_Texture* win_screen = loadTexture("Pics//win_screen.png", renderer);
	SDL_Texture* cloud_fun_light = loadTexture("Pics//cloud_fun_light.png", renderer);
	SDL_Texture* cloud_fun_medium = loadTexture("Pics//cloud_fun_medium.png", renderer);
	SDL_Texture* cloud_fun_hard = loadTexture("Pics//cloud_fun_hard.png", renderer);
	SDL_Texture* cloud_hunger_light = loadTexture("Pics//cloud_hunger_light.png", renderer);
	SDL_Texture* cloud_hunger_medium = loadTexture("Pics//cloud_hunger_medium.png", renderer);
	SDL_Texture* cloud_hunger_hard = loadTexture("Pics//cloud_hunger_hard.png", renderer);
	SDL_Texture* cloud_energy_light = loadTexture("Pics//cloud_energy_light.png", renderer);
	SDL_Texture* cloud_energy_medium = loadTexture("Pics//cloud_energy_medium.png", renderer);
	SDL_Texture* cloud_energy_hard = loadTexture("Pics//cloud_energy_hard.png", renderer);
	SDL_Texture* menu_background = loadTexture("Pics//menu_background.png", renderer);
	SDL_Texture* intro = loadTexture("Pics//intro.png", renderer);
	SDL_Texture* LMS = loadTexture("Pics//LMS1.png", renderer);
	SDL_Texture* shop = loadTexture("Pics//shop.png", renderer);
	// Проверка
	if (!background || !background_miem || !image || !student_pic_full || !student_pic_tired || !smile || !pizza || !lightning ||
		!button_purple_hunger || !button_purple_fun || !button_purple_energy || !button_purple_quit || !button_purple_play || !end_screen || !cloud_fun_light || !cloud_fun_medium || !cloud_fun_hard ||
		!cloud_energy_light || !cloud_energy_medium || !cloud_energy_hard || !cloud_hunger_light || !cloud_hunger_medium || !cloud_hunger_hard ||
		!menu_background || !button_purple_menu || !button_purple_save || !gold || !intro || !LMS || !shop || !books || !win_screen)
	{
		SDL_DestroyTexture(background);
		SDL_DestroyTexture(background_miem);
		SDL_DestroyTexture(image);
		SDL_DestroyTexture(pizza);
		SDL_DestroyTexture(smile);
		SDL_DestroyTexture(lightning);
		SDL_DestroyTexture(gold);
		SDL_DestroyTexture(books);
		SDL_DestroyTexture(student_pic_full);
		SDL_DestroyTexture(student_pic_tired);
		SDL_DestroyTexture(button_purple_hunger);
		SDL_DestroyTexture(button_purple_fun);
		SDL_DestroyTexture(button_purple_energy);
		SDL_DestroyTexture(button_purple_play);
		SDL_DestroyTexture(button_purple_quit);
		SDL_DestroyTexture(button_purple_menu);
		SDL_DestroyTexture(end_screen);
		SDL_DestroyTexture(win_screen);
		SDL_DestroyTexture(cloud_fun_light);
		SDL_DestroyTexture(cloud_fun_medium);
		SDL_DestroyTexture(cloud_fun_hard);
		SDL_DestroyTexture(cloud_energy_light);
		SDL_DestroyTexture(cloud_energy_medium);
		SDL_DestroyTexture(cloud_energy_hard);
		SDL_DestroyTexture(cloud_hunger_light);
		SDL_DestroyTexture(cloud_hunger_medium);
		SDL_DestroyTexture(cloud_hunger_hard);
		SDL_DestroyTexture(menu_background);
		SDL_DestroyTexture(intro);
		SDL_DestroyTexture(LMS);
		SDL_DestroyTexture(shop);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	//Font working beginnig
	TTF_Font* font = TTF_OpenFont("Fonts//sample.ttf", 64);
	if (font == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_OpenFont");
	}

	SDL_Color color_black = { 0, 0, 0, 255 };
	SDL_Color color_gold = { 249, 166, 2, 255 };
	SDL_Color color_white = { 255, 255, 255, 255 };
	SDL_Color color_red = { 194, 24, 7, 255 };
	SDL_Texture* numb_red[10];
	SDL_Texture* numb_black[10];
	for (int i = 0; i < 10; i++)
	{
		numb_red[i] = renderText(to_string(i), font, color_red, renderer);
	}
	for (int i = 0; i < 10; i++)
	{
		numb_black[i] = renderText(to_string(i), font, color_black, renderer);
	}
	SDL_Texture* slash = renderText( "/" , font, color_black, renderer);
	TTF_CloseFont(font);

	// Структура события
	SDL_Event e;

	auto start1 = chrono::system_clock::now();
	auto start2 = chrono::system_clock::now();
	auto start3 = chrono::system_clock::now();
	auto start4 = chrono::system_clock::now();
	auto ct_gold_h = chrono::system_clock::now();
	auto ct_gold_f = chrono::system_clock::now();
	auto ct_gold_e = chrono::system_clock::now();

	int number_width_of_hunger, number_width_of_fun, number_width_of_energy, number_width_of_gold, number_width_of_knowledge;

	int mouse_x = 0;
	int mouse_y = 0;

	bool menu = true;
	bool game = false;
	bool freeze = true;		//intro message
	bool bool_shop = false;
	while (menu) {
		ifstream savein("save.txt"); // (savings file)
		if (!savein.is_open()) { // если файл не открыт (same stats order as in the class of character)
			ofstream saveout("save.txt");
			saveout << 100 << "\n" << 100 << "\n" << 100 << "\n" << 0 << "\n" << 0 << "\n";
			saveout.close();
		}
		string str;
		getline(savein, str);
		int start_hunger = atoi(str.c_str());
		getline(savein, str);
		int start_fun = atoi(str.c_str());
		getline(savein, str);
		int start_energy = atoi(str.c_str());
		getline(savein, str);
		int start_gold = atoi(str.c_str());
		getline(savein, str);
		int start_knowledge = atoi(str.c_str());
		savein.close();
		Student.set_hunger() = start_hunger;
		Student.set_fun() = start_fun;
		Student.set_energy() = start_energy;
		Student.set_gold() = start_gold;
		Student.set_knowledge() = start_knowledge;

		int CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT;
		SDL_RenderClear(renderer);
		SDL_GetWindowSize(window, &CURRENT_WINDOW_WIDTH, &CURRENT_WINDOW_HEIGHT);
		renderTexture_m(menu_background, renderer, 0, 0, CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);
		renderTexture_m(button_purple_play, renderer, 160, CURRENT_WINDOW_HEIGHT - 290, 190, 60);
		renderTexture_m(button_purple_quit, renderer, 160, CURRENT_WINDOW_HEIGHT - 190, 190, 60);
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				menu = false;
			if (e.type == SDL_MOUSEMOTION)
			{
				SDL_GetMouseState(&mouse_x, &mouse_y);
			}
			if ((mouse_x >= 160) && (mouse_y >= (CURRENT_WINDOW_HEIGHT - 290)) && (mouse_x <= 350) && (mouse_y <= (CURRENT_WINDOW_HEIGHT - 230)))
				if (e.type == SDL_MOUSEBUTTONDOWN)
					game = true;
			if ((mouse_x >= 160) && (mouse_y >= (CURRENT_WINDOW_HEIGHT - 190)) && (mouse_x <= 350) && (mouse_y <= (CURRENT_WINDOW_HEIGHT - 130)))
				if (e.type == SDL_MOUSEBUTTONDOWN)
					menu = false;
		}
		if (game == true); {
			while (game)	//Условие бексонечного цикла
			{
				SDL_RenderClear(renderer);
				SDL_GetWindowSize(window, &CURRENT_WINDOW_WIDTH, &CURRENT_WINDOW_HEIGHT);
				renderTexture_m(background_miem, renderer, 0, 0, CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);

				check_width_for_texture(Student.get_hunger(), number_width_of_hunger);
				check_width_for_texture(Student.get_fun(), number_width_of_fun);
				check_width_for_texture(Student.get_energy(), number_width_of_energy);
				check_width_for_texture(Student.get_gold(), number_width_of_gold);
				check_width_for_texture(Student.get_knowledge(), number_width_of_knowledge);

				if (freeze == false) {
					if (Student.get_hunger() < 10)
						renderTexture_m(numb_red[Student.get_hunger()], renderer, CURRENT_WINDOW_WIDTH - 65, 40, 15, 35);
					else
						if ((Student.get_hunger() > 9) && (Student.get_hunger() < 100)){
							if (number_color(Student.get_hunger()) == 0) {
								renderTexture_m(numb_red[Student.get_hunger() / 10], renderer, CURRENT_WINDOW_WIDTH - 65, 40, 15, 35);
								renderTexture_m(numb_red[Student.get_hunger() % 10], renderer, CURRENT_WINDOW_WIDTH - 50, 40, 15, 35);
							}
							else { 
								renderTexture_m(numb_black[Student.get_hunger() / 10], renderer, CURRENT_WINDOW_WIDTH - 65, 40, 15, 35);
								renderTexture_m(numb_black[Student.get_hunger() % 10], renderer, CURRENT_WINDOW_WIDTH - 50, 40, 15, 35);
							}
						}
						else { 
							renderTexture_m(numb_black[Student.get_hunger() / 100], renderer, CURRENT_WINDOW_WIDTH - 65, 40, 15, 35);
							renderTexture_m(numb_black[(Student.get_hunger() % 100) / 10], renderer, CURRENT_WINDOW_WIDTH - 50, 40, 15, 35);
							renderTexture_m(numb_black[Student.get_hunger() % 10], renderer, CURRENT_WINDOW_WIDTH - 35, 40, 15, 35);
						}
					if (Student.get_fun() < 10)
						renderTexture_m(numb_red[Student.get_fun()], renderer, CURRENT_WINDOW_WIDTH - 65, 80, 15, 35);
					else
						if ((Student.get_fun() > 9) && (Student.get_fun() < 100)) {
							if (number_color(Student.get_fun()) == 0) {
								renderTexture_m(numb_red[Student.get_fun() / 10], renderer, CURRENT_WINDOW_WIDTH - 65, 80, 15, 35);
								renderTexture_m(numb_red[Student.get_fun() % 10], renderer, CURRENT_WINDOW_WIDTH - 50, 80, 15, 35);
							}
							else {
								renderTexture_m(numb_black[Student.get_fun() / 10], renderer, CURRENT_WINDOW_WIDTH - 65, 80, 15, 35);
								renderTexture_m(numb_black[Student.get_fun() % 10], renderer, CURRENT_WINDOW_WIDTH - 50, 80, 15, 35);
							}
						}
						else {
							renderTexture_m(numb_black[Student.get_fun() / 100], renderer, CURRENT_WINDOW_WIDTH - 65, 80, 15, 35);
							renderTexture_m(numb_black[(Student.get_fun() % 100) / 10], renderer, CURRENT_WINDOW_WIDTH - 50, 80, 15, 35);
							renderTexture_m(numb_black[Student.get_fun() % 10], renderer, CURRENT_WINDOW_WIDTH - 35, 80, 15, 35);
						}
					if (Student.get_energy() < 10)
						renderTexture_m(numb_red[Student.get_energy()], renderer, CURRENT_WINDOW_WIDTH - 65, 40, 15, 35);
					else
						if ((Student.get_energy() > 9) && (Student.get_energy() < 100)) {
							if (number_color(Student.get_energy()) == 0) {
								renderTexture_m(numb_red[Student.get_energy() / 10], renderer, CURRENT_WINDOW_WIDTH - 65, 120, 15, 35);
								renderTexture_m(numb_red[Student.get_energy() % 10], renderer, CURRENT_WINDOW_WIDTH - 50, 120, 15, 35);
							}
							else {
								renderTexture_m(numb_black[Student.get_energy() / 10], renderer, CURRENT_WINDOW_WIDTH - 65, 120, 15, 35);
								renderTexture_m(numb_black[Student.get_energy() % 10], renderer, CURRENT_WINDOW_WIDTH - 50, 120, 15, 35);
							}
						}
						else {
							renderTexture_m(numb_black[Student.get_energy() / 100], renderer, CURRENT_WINDOW_WIDTH - 65, 120, 15, 35);
							renderTexture_m(numb_black[(Student.get_energy() % 100) / 10], renderer, CURRENT_WINDOW_WIDTH - 50, 120, 15, 35);
							renderTexture_m(numb_black[Student.get_energy() % 10], renderer, CURRENT_WINDOW_WIDTH - 35, 120, 15, 35);
						}
					if (Student.get_gold() < 10)
						renderTexture_m(numb_black[Student.get_gold()], renderer, 60, 75, 15, 35);
					else
						if ((Student.get_gold() > 9) && (Student.get_gold() < 100)) {
							renderTexture_m(numb_black[Student.get_gold() / 10], renderer, 60, 75, 15, 35);
							renderTexture_m(numb_black[Student.get_gold() % 10], renderer, 75, 75, 15, 35);
						}
						else {
							renderTexture_m(numb_black[Student.get_gold() / 100], renderer, 60, 75, 15, 35);
							renderTexture_m(numb_black[(Student.get_gold() % 100) / 10], renderer, 75, 75, 15, 35);
							renderTexture_m(numb_black[Student.get_gold() % 10], renderer, 90, 75, 15, 35);
						}
					if (Student.get_knowledge() < 10)
						renderTexture_m(numb_black[Student.get_knowledge()], renderer, 60, 115, 15, 35);
					else
						if ((Student.get_knowledge() > 9) && (Student.get_knowledge() < 100)) {
							renderTexture_m(numb_black[Student.get_knowledge() / 10], renderer, 60, 115, 15, 35);
							renderTexture_m(numb_black[Student.get_knowledge() % 10], renderer, 75, 115, 15, 35);
						}
						else {
							renderTexture_m(numb_black[Student.get_knowledge() / 100], renderer, 60, 115, 15, 35);
							renderTexture_m(numb_black[(Student.get_knowledge() % 100) / 10], renderer, 75, 115, 15, 35);
							renderTexture_m(numb_black[Student.get_knowledge() % 10], renderer, 90, 115, 15, 35);
						}
					renderTexture_m(slash, renderer, 107, 115, 15, 35);
					renderTexture_m(numb_black[5], renderer, 122, 115, 15, 35);
					renderTexture_m(numb_black[0], renderer, 137, 115, 15, 35);
					renderTexture_m(numb_black[0], renderer, 152, 115, 15, 35);
				}

				auto end = chrono::system_clock::now();
				if (freeze == false) {
					chrono::duration<double> elapsed_seconds1 = end - start1;
					if (elapsed_seconds1.count() >= 0.6) {
						Student.decrease_hunger();
						start1 = chrono::system_clock::now();
					}
					chrono::duration<double> elapsed_seconds2 = end - start2;
					if (elapsed_seconds2.count() >= 0.7) {
						Student.decrease_fun();
						start2 = chrono::system_clock::now();
					}
					chrono::duration<double> elapsed_seconds3 = end - start3;
					if (elapsed_seconds3.count() >= 0.9) {
						Student.decrease_energy();
						start3 = chrono::system_clock::now();
					}
					chrono::duration<double> elapsed_seconds4 = end - start4;
					if (elapsed_seconds4.count() >= 30) {
						if (Student.get_gold() > 49)
							Student.decrease_gold(50);
						else Student.decrease_gold(Student.get_gold());
						start4 = chrono::system_clock::now();
					}
				}

				if (Student.get_hunger() < 21 || Student.get_fun() < 21 || Student.get_energy() < 21)
					renderTexture_m(student_pic_tired, renderer, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 50, CURRENT_WINDOW_HEIGHT - 370, 100, 300);
				else
					renderTexture_m(student_pic_full, renderer, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 50, CURRENT_WINDOW_HEIGHT - 370, 100, 300);

				//Stats markers
				if (freeze == false) {
					renderTexture_m(pizza, renderer, CURRENT_WINDOW_WIDTH - 104, 43, 29, 29);
					renderTexture_m(smile, renderer, CURRENT_WINDOW_WIDTH - 104, 83, 29, 29);
					renderTexture_m(lightning, renderer, CURRENT_WINDOW_WIDTH - 104, 123, 29, 29);
					renderTexture_m(gold, renderer, 20, 75, 30, 30);
					renderTexture_m(books, renderer, 20, 115, 30, 30);
				}

				if (bool_shop == true)
					renderTexture_m(shop, renderer, CURRENT_WINDOW_WIDTH - 320, CURRENT_WINDOW_HEIGHT - 262, 300, 180);

				//Buttons draw
				renderTexture_m(button_purple_hunger, renderer, ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2), CURRENT_WINDOW_HEIGHT - 62, 120, 40);
				renderTexture_m(button_purple_fun, renderer, ceil(float(CURRENT_WINDOW_WIDTH) / 4) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2), CURRENT_WINDOW_HEIGHT - 62, 120, 40);
				renderTexture_m(button_purple_energy, renderer, ceil((float(CURRENT_WINDOW_WIDTH) / 4)*2) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2), CURRENT_WINDOW_HEIGHT - 62, 120, 40);
				renderTexture_m(LMS, renderer, ceil((float(CURRENT_WINDOW_WIDTH) / 4) * 3) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120)/2), CURRENT_WINDOW_HEIGHT - 62, 120, 40);
				renderTexture_m(button_purple_menu, renderer, 20, 15, 120, 40);
				renderTexture_m(button_purple_save, renderer, 160, 15, 120, 40);

				int end_h = Student.get_hunger();
				int end_f = Student.get_fun();
				int end_e = Student.get_energy();

				renderCloud(cloud_hunger_light, renderer, end_h, 40, 60, ceil(float(CURRENT_WINDOW_WIDTH) / 2) + 25, CURRENT_WINDOW_HEIGHT - 370, 175, 60);
				renderCloud(cloud_hunger_medium, renderer, end_h, 20, 40, ceil(float(CURRENT_WINDOW_WIDTH) / 2) + 25, CURRENT_WINDOW_HEIGHT - 370, 175, 60);
				renderCloud(cloud_hunger_hard, renderer, end_h, 0, 20, ceil(float(CURRENT_WINDOW_WIDTH) / 2) + 25, CURRENT_WINDOW_HEIGHT - 370, 175, 60);
				renderCloud(cloud_fun_light, renderer, end_f, 40, 60, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 220, CURRENT_WINDOW_HEIGHT - 370, 175, 60);
				renderCloud(cloud_fun_medium, renderer, end_f, 20, 40, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 220, CURRENT_WINDOW_HEIGHT - 370, 175, 60);
				renderCloud(cloud_fun_hard, renderer, end_f, 0, 20, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 220, CURRENT_WINDOW_HEIGHT - 370, 175, 60);
				renderCloud(cloud_energy_light, renderer, end_e, 40, 60, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 97, CURRENT_WINDOW_HEIGHT - 440, 175, 70);
				renderCloud(cloud_energy_medium, renderer, end_e, 20, 40, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 97, CURRENT_WINDOW_HEIGHT - 440, 175, 70);
				renderCloud(cloud_energy_hard, renderer, end_e, 0, 20, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 97, CURRENT_WINDOW_HEIGHT - 440, 175, 70);

				if (freeze == true) {
					renderTexture_m(intro, renderer, 10, 60, CURRENT_WINDOW_WIDTH - 20, CURRENT_WINDOW_HEIGHT - 130);
				}

				// Обработка событий
				while (SDL_PollEvent(&e))
				{
					if (e.type == SDL_QUIT) {
						game = false;
						menu = false;
						freeze = false;
					}

					if (e.type == SDL_KEYDOWN)
					{
						switch (e.key.keysym.sym)
						{
						case SDLK_ESCAPE:
							if (freeze == false) {
								game = false;
							}
							freeze = false; 
							break;
						default:
							break;
						}
					}

					if (e.type == SDL_KEYDOWN) {
						freeze = false;
					}

					if (e.type == SDL_MOUSEMOTION)
					{
						SDL_GetMouseState(&mouse_x, &mouse_y);
					}

					if (e.type == SDL_MOUSEBUTTONDOWN) {
						freeze = false;
					}
					if ((mouse_x >= 160) && (mouse_y >= 15) && (mouse_x <= 280) && (mouse_y <= 55))
						if (e.type == SDL_MOUSEBUTTONDOWN) {
							ofstream saveout("save.txt", ios_base::trunc);
							saveout << Student.get_hunger() << "\n" << Student.get_fun() << "\n" << Student.get_energy() << "\n" << Student.get_gold() << "\n" << Student.get_knowledge() << "\n";
							saveout.close();
						}
					if ((mouse_x >= 20) && (mouse_y >= 15) && (mouse_x <= 140) && (mouse_y <= 55))
						if (e.type == SDL_MOUSEBUTTONDOWN) {
							game = false;
						}
					if ((mouse_x >= ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2)) && (mouse_y >= (CURRENT_WINDOW_HEIGHT - 62)) && (mouse_x <= ceil((((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2) + 120)) && (mouse_y <= (CURRENT_WINDOW_HEIGHT - 22)))
						if (e.type == SDL_MOUSEBUTTONDOWN)
							if (Student.get_hunger() < 93) {
								Student.increase_hunger(8);
								chrono::duration<double> elapsed_seconds_h = end - ct_gold_h;
								if (elapsed_seconds_h.count() >= 1.5) {
									Student.increase_gold(6);
									ct_gold_h = chrono::system_clock::now();
								}
							}
							else {
								Student.increase_hunger(100 - Student.get_hunger());
								chrono::duration<double> elapsed_seconds_h = end - ct_gold_h;
								if (elapsed_seconds_h.count() >= 1.5) {
									Student.increase_gold(6);
									ct_gold_h = chrono::system_clock::now();
								}
							}
					if ((mouse_x >= (ceil(float(CURRENT_WINDOW_WIDTH) / 4) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2))) && (mouse_y >= (CURRENT_WINDOW_HEIGHT - 62)) && (mouse_x <= ceil((float(CURRENT_WINDOW_WIDTH) / 4) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2) + 120)) && (mouse_y <= (CURRENT_WINDOW_HEIGHT - 22)))
						if (e.type == SDL_MOUSEBUTTONDOWN)
							if (Student.get_fun() < 89) {
								Student.increase_fun(10);
								chrono::duration<double> elapsed_seconds_f = end - ct_gold_f;
								if (elapsed_seconds_f.count() >= 2) {
									Student.increase_gold(8);
									ct_gold_f = chrono::system_clock::now();
								}
							}
							else {
								Student.increase_fun(100 - Student.get_fun());
								chrono::duration<double> elapsed_seconds_f = end - ct_gold_f;
								if (elapsed_seconds_f.count() >= 0.75) {
									Student.increase_gold(5);
									ct_gold_f = chrono::system_clock::now();
								}
							}
					if ((mouse_x >= ceil((float(CURRENT_WINDOW_WIDTH) / 4) * 2) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2)) && (mouse_y >= (CURRENT_WINDOW_HEIGHT - 62)) && (mouse_x <= (ceil((float(CURRENT_WINDOW_WIDTH) / 4) * 2) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2) + 120)) && (mouse_y <= (CURRENT_WINDOW_HEIGHT - 22)))
						if (e.type == SDL_MOUSEBUTTONDOWN)
							if (Student.get_energy() < 91) {
								Student.increase_energy(12);
								chrono::duration<double> elapsed_seconds_e = end - ct_gold_e;
								if (elapsed_seconds_e.count() >= 1) {
									Student.increase_gold(6);
									ct_gold_e = chrono::system_clock::now();
								}
							}
							else {
								Student.increase_energy(100 - Student.get_energy());
								chrono::duration<double> elapsed_seconds_e = end - ct_gold_e;
								if (elapsed_seconds_e.count() >= 1.25) {
									Student.increase_gold(7);
									ct_gold_e = chrono::system_clock::now();
								}
							}
					if ((mouse_x >= ceil((float(CURRENT_WINDOW_WIDTH) / 4) * 3) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2)) && (mouse_y >= CURRENT_WINDOW_HEIGHT - 62) && (mouse_x <= (ceil((float(CURRENT_WINDOW_WIDTH) / 4) * 3) + ceil(((float(CURRENT_WINDOW_WIDTH) / 4) - 120) / 2) + 120)) && (mouse_y <= (CURRENT_WINDOW_HEIGHT - 22)))
						if (e.type == SDL_MOUSEBUTTONDOWN)
							if (bool_shop == false)
								bool_shop = true;
							else
								bool_shop = false;
					if (bool_shop == true) {
						if ((mouse_x >= CURRENT_WINDOW_WIDTH - 320) && (mouse_y >= CURRENT_WINDOW_HEIGHT - 262) && (mouse_x <= CURRENT_WINDOW_WIDTH - 222) && (mouse_y <= CURRENT_WINDOW_HEIGHT - 174))
							if (e.type == SDL_MOUSEBUTTONDOWN) 	//конспекты
								if (Student.get_gold() > 3)
									if (Student.get_knowledge() < 491) {
										Student.decrease_gold(4);
										Student.increase_knowledge(10);
									}
									else {
										Student.decrease_gold(4);
										Student.increase_knowledge(500 - Student.get_knowledge());
									}
						if ((mouse_x >= CURRENT_WINDOW_WIDTH - 218) && (mouse_y >= CURRENT_WINDOW_HEIGHT - 262) && (mouse_x <= CURRENT_WINDOW_WIDTH - 122) && (mouse_y <= CURRENT_WINDOW_HEIGHT - 174))
							if (e.type == SDL_MOUSEBUTTONDOWN) 	//эл учебник
								if (Student.get_gold() > 4) 
									if (Student.get_knowledge() < 488) {
										Student.decrease_gold(5);
										Student.increase_knowledge(13);
									}
									else {
										Student.decrease_gold(5);
										Student.increase_knowledge(500 - Student.get_knowledge());
									}
						if ((mouse_x >= CURRENT_WINDOW_WIDTH - 118) && (mouse_y >= CURRENT_WINDOW_HEIGHT - 262) && (mouse_x <= CURRENT_WINDOW_WIDTH - 20) && (mouse_y <= CURRENT_WINDOW_HEIGHT - 174))
							if (e.type == SDL_MOUSEBUTTONDOWN) 	//печат учебник
								if (Student.get_gold() > 5) 
									if (Student.get_knowledge() < 485) {
										Student.decrease_gold(6);
										Student.increase_knowledge(16);
									}
									else {
										Student.decrease_gold(6);
										Student.increase_knowledge(500 - Student.get_knowledge());
									}
						if ((mouse_x >= CURRENT_WINDOW_WIDTH - 320) && (mouse_y >= CURRENT_WINDOW_HEIGHT - 170) && (mouse_x <= CURRENT_WINDOW_WIDTH - 222) && (mouse_y <= CURRENT_WINDOW_HEIGHT - 82))
							if (e.type == SDL_MOUSEBUTTONDOWN) 	// лекция
								if (Student.get_gold() > 7) 
									if (Student.get_knowledge() < 477) {
										Student.decrease_gold(8);
										Student.increase_knowledge(24);
									}
									else {
										Student.decrease_gold(8);
										Student.increase_knowledge(500 - Student.get_knowledge());
									}
						if ((mouse_x >= CURRENT_WINDOW_WIDTH - 218) && (mouse_y >= CURRENT_WINDOW_HEIGHT - 170) && (mouse_x <= CURRENT_WINDOW_WIDTH - 122) && (mouse_y <= CURRENT_WINDOW_HEIGHT - 82))
							if (e.type == SDL_MOUSEBUTTONDOWN) 	//вебинар
								if (Student.get_gold() > 9)
									if (Student.get_knowledge() < 471) {
										Student.decrease_gold(10);
										Student.increase_knowledge(30);
									}
									else {
										Student.decrease_gold(10);
										Student.increase_knowledge(500 - Student.get_knowledge());
									}
						if ((mouse_x >= CURRENT_WINDOW_WIDTH - 118) && (mouse_y >= CURRENT_WINDOW_HEIGHT - 170) && (mouse_x <= CURRENT_WINDOW_WIDTH - 20) && (mouse_y <= CURRENT_WINDOW_HEIGHT - 82))
							if (e.type == SDL_MOUSEBUTTONDOWN) 	//курс
								if (Student.get_gold() > 14)
									if (Student.get_knowledge() < 453) {
										Student.decrease_gold(15);
										Student.increase_knowledge(48);
									}
									else {
										Student.decrease_gold(15);
										Student.increase_knowledge(500 - Student.get_knowledge());
									}
					}
				}
				if ((end_h < 1) || (end_f < 1) || (end_e < 1))
				{
					freeze = true;
					bool death = true;
					while (death)
					{
						SDL_RenderClear(renderer);
						SDL_GetWindowSize(window, &CURRENT_WINDOW_WIDTH, &CURRENT_WINDOW_HEIGHT);
						renderTexture_m(background_miem, renderer, 0, 0, CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);
						renderTexture_m(end_screen, renderer, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 150, ceil(float(CURRENT_WINDOW_HEIGHT) / 2) - 200, 300, 400);
						while (SDL_PollEvent(&e))
						{
							if (e.type == SDL_QUIT) {
								menu = false;
								game = false;
								death = false;
							}
							if (e.type == SDL_KEYDOWN)
							{
								switch (e.key.keysym.sym)
								{
								case SDLK_RETURN:
									game = false;
									death = false;
									break;
								case SDLK_ESCAPE:
									game = false;
									death = false;
									break;
								}
							}
							if (e.type == SDL_MOUSEBUTTONDOWN) {
								game = false;
								death = false;
							}
						}
						SDL_RenderPresent(renderer);
						SDL_Delay(3);
					}
					Student.set_hunger() = 100;
					Student.set_fun() = 100;
					Student.set_energy() = 100;
					Student.set_gold() = 0;
					Student.set_knowledge() = 0;
					ofstream saveout("save.txt", ios_base::trunc);
					saveout << 100 << "\n" << 100 << "\n" << 100 << "\n" << 0 << "\n" << 0 << "\n";
					saveout.close();
				}

				if (Student.get_knowledge() >= 500)
				{
					freeze = true;
					bool win = true;
					while (win)
					{
						SDL_RenderClear(renderer);
						SDL_GetWindowSize(window, &CURRENT_WINDOW_WIDTH, &CURRENT_WINDOW_HEIGHT);
						renderTexture_m(background_miem, renderer, 0, 0, CURRENT_WINDOW_WIDTH, CURRENT_WINDOW_HEIGHT);
						renderTexture_m(win_screen, renderer, ceil(float(CURRENT_WINDOW_WIDTH) / 2) - 250, ceil(float(CURRENT_WINDOW_HEIGHT) / 2) - 250, 500, 500);
						while (SDL_PollEvent(&e))
						{
							if (e.type == SDL_QUIT) {
								menu = false;
								game = false;
								win = false;
							}
							if (e.type == SDL_KEYDOWN)
							{
								switch (e.key.keysym.sym)
								{
								case SDLK_RETURN:
									game = false;
									win = false;
									break;
								case SDLK_ESCAPE:
									game = false;
									win = false;
									break;
								}
							}
							if (e.type == SDL_MOUSEBUTTONDOWN) {
								game = false;
								win = false;
							}
						}
						SDL_RenderPresent(renderer);
						SDL_Delay(3);
					}
					Student.set_hunger() = 100;
					Student.set_fun() = 100;
					Student.set_energy() = 100;
					Student.set_gold() = 0;
					Student.set_knowledge() = 0;
					ofstream saveout("save.txt", ios_base::trunc);
					saveout << 100 << "\n" << 100 << "\n" << 100 << "\n" << 0 << "\n" << 0 << "\n";
					saveout.close();
				}
				SDL_RenderPresent(renderer);
				SDL_Delay(3);
			}
		}
		SDL_RenderPresent(renderer);
		SDL_Delay(3);
	}
	//ShowConsole();
	for (int i = 0; i < 10; i++) { SDL_DestroyTexture(numb_red[i]); }
	for (int i = 0; i < 10; i++) { SDL_DestroyTexture(numb_black[i]); }
	SDL_DestroyTexture(slash);
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(background_miem);
	SDL_DestroyTexture(image);
	SDL_DestroyTexture(pizza);
	SDL_DestroyTexture(smile);
	SDL_DestroyTexture(lightning);
	SDL_DestroyTexture(gold);
	SDL_DestroyTexture(books);
	SDL_DestroyTexture(student_pic_full);
	SDL_DestroyTexture(student_pic_tired);
	SDL_DestroyTexture(button_purple_hunger);
	SDL_DestroyTexture(button_purple_fun);
	SDL_DestroyTexture(button_purple_energy);
	SDL_DestroyTexture(button_purple_play);
	SDL_DestroyTexture(button_purple_quit);
	SDL_DestroyTexture(button_purple_menu);
	SDL_DestroyTexture(button_purple_save);
	SDL_DestroyTexture(end_screen);
	SDL_DestroyTexture(win_screen);
	SDL_DestroyTexture(cloud_fun_light);
	SDL_DestroyTexture(cloud_fun_medium);
	SDL_DestroyTexture(cloud_fun_hard);
	SDL_DestroyTexture(cloud_energy_light);
	SDL_DestroyTexture(cloud_energy_medium);
	SDL_DestroyTexture(cloud_energy_hard);
	SDL_DestroyTexture(cloud_hunger_light);
	SDL_DestroyTexture(cloud_hunger_medium);
	SDL_DestroyTexture(cloud_hunger_hard);
	SDL_DestroyTexture(menu_background);
	SDL_DestroyTexture(intro);
	SDL_DestroyTexture(LMS);
	SDL_DestroyTexture(shop);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return(0);
}