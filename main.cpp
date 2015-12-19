#include <iostream>
#include <vector>
#include <limits>
#include <ctime>

using namespace std;

typedef vector<uint8_t> Board; //игровое поле


//игровые настройки
const uint8_t POLE = 3; //размер игрового поля (от четырех клеток начинает призадумываться над ходами)
const int MAX_LEVEL = 20; //сложность игры (easy 3..17; normal 18..40)
const bool FIRST_PLAYER = true; //кто первый ходит
const uint8_t PLAYER = 1; //крестик
const uint8_t COMP = PLAYER + POLE; //нолик



//рейтинг хода
class Rate
{
public:
	Rate()
	{
		stepWin = numeric_limits<unsigned int>::max() - 1; //максимальное значение нужно для алгоритма
		stepLoss = numeric_limits<unsigned int>::max() - 1;
		nWin = 0;
		nLoss = 0;
	}


	//через сколько ходов выигрыш
	unsigned int getStepWin()
	{
		return stepWin;
	}


	//через сколько ходов проигрыш
	unsigned int getStepLoss()
	{
		return stepLoss;
	}


	//сколько сценариев выигрыша на шаге stepWin
	unsigned int getNWin()
	{
		return nWin;
	}


	//сколько сценариев проигрыша
	unsigned int getNLoss()
	{
		return nLoss;
	}


	//влючает в расчет новый rate
	void add(Rate &&rate)
	{
		//если добавляется аналогичный rate, то увеличить счетчик сценариев выигрыша
		if (rate.stepWin == this->stepWin)
		++this->nWin;

		//если добавляется лучший (с меньшим кол-вом шагов до выигрыша), то заменить
		if (rate.stepWin < this->stepWin) {
			this->stepWin = rate.stepWin;
			this->nWin = 1;
		}

		//если меньше шагов до проигрыша, то заменить
		if (rate.stepLoss < this->stepLoss) {
			this->stepLoss = rate.stepLoss;
		}

		this->nLoss += rate.getNLoss(); //общее кол-во сценариев проигрыша
	}


	//увеличить число ходов (stepWin, stepLoss)
	void incStep()
	{
		if (stepWin < numeric_limits<unsigned int>::max() - 1)
			++stepWin;
		if (stepLoss < numeric_limits<unsigned int>::max() - 1)
			++stepLoss;
	}


	//сравнивает два рейтинга (-1 - лучше this; 1 - лучше rate; 0 - равны)
	int8_t compare(Rate &rate) const
	{
		//сравнить на максимальный ход до проигрыша
		if (rate.getStepLoss() > this->stepLoss)
			return 1;

		//сравнить минимальный ход до выигрыша
		if ((rate.getStepLoss() == this->stepLoss) && (rate.getStepWin() < this->stepWin))
			return 1;

		//сравнить максимальные сценарии выигрыша
		if ((rate.getStepLoss() == this->stepLoss) && (rate.getStepWin() == this->stepWin) && (rate.getNWin() > this->nWin))
			return 1;

		//сравнить минимальные сценарии проигрыша
		if ((rate.getStepLoss() == this->stepLoss) && (rate.getStepWin() == this->stepWin) && (rate.getNWin() == this->nWin) && rate.getNLoss() < this->nLoss)
			return 1;

		//проверить на равенство
		if ((rate.getStepLoss() == this->stepLoss) && (rate.getStepWin() == this->stepWin) && (rate.getNWin() == this->nWin) && rate.getNLoss() == this->nLoss)
			return 0;

		return -1;
	}


	void initStepWin()
	{
		stepWin = 1;
		nWin = 1;
	}


	void initStepLoss()
	{
		stepLoss = 1;
		nLoss = 1;
	}


private:
	unsigned int stepWin; //через сколько ходов выигрыш
	unsigned int stepLoss; //через сколько ходов проигрыш
	unsigned int nWin; //сколько сценариев выигрыша на шаге stepWin
	unsigned int nLoss; //сколько сценариев проигрыша
};



//проверяет закончилась ли игра (0-игра продолжается,1-победил компьютер,2-победил пользователь,3-ничья)
uint8_t isFinish(const Board &board)
{
	//горизонтали и вертикали
	for (uint8_t i = 0; i < POLE ; ++i) {
		int sum1 = 0; int sum2 = 0;
		for (uint8_t j = 0; j < POLE ; ++j) {
			sum1 +=board[i * POLE + j];
			sum2 +=board[i + j * POLE];
		}
		if ((sum1 == COMP * POLE) || (sum2 == COMP * POLE)) return 1; //победа
		if ((sum1 == PLAYER * POLE) || (sum2 == PLAYER * POLE)) return 2; //проигрыш
	}

	//диагонали
	int sum1 = 0; int sum2 = 0;
	for (uint8_t i = 0; i < POLE ; ++i) {
		sum1 +=board[i + i * POLE];
		sum2 +=board[(i + 1) * POLE - i - 1];
		if ((sum1 == COMP * POLE) || (sum2 == COMP * POLE)) return 1; //победа
		if ((sum1 == PLAYER * POLE) || (sum2 == PLAYER * POLE)) return 2; //проигрыш
	}

	for (unsigned int i = 0; i < POLE * POLE ; ++i)
		if (board[i] == 0) return 0; //игра не закончена

	return 3; //ничья
}



//возвращает рейтинг для указанной позиции pos на игровой доске board для игрока player
Rate rating(Board board,  uint8_t pos, bool player, unsigned int level)
{
	//сделаем ход на поле
	if (player) board[pos] = PLAYER;
	else board[pos] = COMP;

	//закончилась ли игра
	int res = isFinish(board);
	Rate result;
	if (res == 1)
		result.initStepWin();
	else if (res == 2)
		result.initStepLoss();
	if (res > 0)
		return result;

	//просчитать рейтинг в других клетках
	if (level < MAX_LEVEL) for (uint8_t i = 0; i < POLE * POLE ; ++i) {
		if (board[i] == 0) {
			//выбрать сценарий с самым минимальным кол-вом ходом для выигрыша/проигрыша
			result.add(rating(board, i, !player, ++level));
		}
	}

	//увеличить счётчик ходов при возврате
	result.incStep();
	return result;
}



//выводит на экран игровое поле в ASCII
void printBoard(Board &board)
{
	char simv[14];
	simv[0] = 201; simv[1] = 187; simv[2] = 188; simv[3] = 200; simv[4] = 203; simv[5] = 185; simv[6] = 202;
	simv[7] = 204; simv[8] = 206; simv[9] = 205; simv[10] = 186; simv[11] = 88; simv[12] = 79; simv[13] = 0;

	//ASCII art
	cout << "\t\tGAME BOARD\n\t\t ";
	for (int i = 0; i < POLE; ++i) cout << " " << i; cout << "\n";
	cout << "\t\t " << simv[0]; for (uint8_t i = 0; i < POLE - 1; ++i) cout << simv[9] << simv[4]; cout << simv[9] << simv[1] << "\n";
	for (int ii = 0; ii < POLE - 1; ++ii) {
		cout << "\t\t" << ii << simv[10];
		for (int i = 0; i < POLE; ++i) {
			if (board[i + POLE * ii] == PLAYER) cout << simv[11];
			else if (board[i + POLE * ii] == COMP) cout << simv[12];
			else if (board[i + POLE * ii] == 0) cout << simv[13];
			cout << simv[10];
		}
		cout << "\n";
		cout << "\t\t " << simv[7]; for (int i = 0; i < POLE - 1; ++i) cout << simv[9] << simv[8]; cout << simv[9] << simv[5] << "\n";
	}
	cout << "\t\t" << POLE - 1 << simv[10];
	for (int i = 0; i < POLE; ++i) {
		if (board[i + POLE * (POLE - 1)] == PLAYER) cout << simv[11];
		else if (board[i + POLE * (POLE - 1)] == COMP) cout << simv[12];
		else if (board[i + POLE * (POLE - 1)] == 0) cout << simv[13];
		cout << simv[10];
	}
	cout << "\n";
	cout << "\t\t " << simv[3]; for (int i = 0; i < POLE - 1; ++i) cout << simv[9] << simv[6]; cout << simv[9] << simv[2] << "\n";
}



//один раунд игры
void gameRound(Board &board)
{
	system("cls"); //очистка экрана
	printBoard(board); //вывод игрового поля

	//чтение хода игрока
	cout <<  "\nYour turn (row,col) (eg: 1,1): ";
	int i, j;
	bool correctData = false; //ввели верные данные
	while (!correctData) {
		//проверка на корректный ввод данных
		while (scanf("%d,%d", &i, &j) != 2) {
			fflush(stdin); //сброс буфера чтения
			cout << "Wrong format. Enter row,col (eg: 1,1): ";
		}
		//проверка на выход за границы доски и свободна ли выбранная клетка
		if (((i * POLE + j) < POLE * POLE) && (board[i * POLE + j] == 0))
			correctData = true;
		else
			cout << "Select wrong cell. Select real and empty cell. Enter row,col (eg: 1,1): ";
	}
	board[i * POLE + j] = PLAYER; //проставить ход игрока

	//выбрать оптимальный ход компьютера
	Board bestSteps; //список вариантов лучших ходов
	bestSteps.reserve(POLE * POLE);
	Rate bestRate; //последний найденный лучший рейтинг
	for (unsigned int i = 0; i < POLE * POLE ; ++i) { //расчёт рейтинга свободных клеток
		if (board[i] != 0) continue;
		//выбор лучшего рейтинга
		Rate tmpRate = rating(board, i, false, 0);
		int cmp = bestRate.compare(tmpRate);
		if ((bestSteps.size() == 0) || (cmp == 1)) { //если найденый рейтинг лучше, чем запомненный. Первое условие для случая, когда последний найденный рейтинг ещё не существует.
			bestRate = tmpRate;
			bestSteps.clear();
			bestSteps.push_back(i);
		}
		if (cmp == 0)
		bestSteps.push_back(i); //если найденный рейтинг такой же, то запомнить клетку в список
	}

	if (bestSteps.size() > 0)
		board[bestSteps[rand() % bestSteps.size()]] = COMP;
}

int main()
{
	srand(time(NULL)); //рандомайзер

	Board board(POLE * POLE); //клетки поля (0-пустое; PLAYER-крестик, COMP-нолик)

	if (!FIRST_PLAYER) //если первый ход у компьютера
		board[rand() % POLE * POLE] = COMP;

	//цикл игры
	while (isFinish(board) == 0)
		gameRound(board);

	system("cls"); //очистка экрана
	printBoard(board); //вывод игрового поля

	uint8_t res = isFinish(board);
	if (res == 1)
		cout << "Computer WIN";
	else if (res == 2)
		cout << "You WIN";
	else
		cout << "Dead heat";

	return EXIT_SUCCESS;
}