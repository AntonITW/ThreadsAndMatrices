#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <windows.h>
#include <functional>
#include <mutex>

using namespace std;

//Объявление мютекса
mutex m;

static const int MATRIX_SIZE = 1000; //Размер матрицы
static const int THREADS_NUMBER = 5; //Количество выполняемых потоков
static const int N_EXECUTIONS = 10;  //Количество выполняемых перемножений
int const s = 0;
int const e = 100; //Ранжирование случайных чисел в матрицах

struct Matrix {
	int** elements; //Ссылка на матрицу

	//Инициализация матрицы с 0 во всех ячейках
	void fill_zeros() {
		elements = new int* [MATRIX_SIZE];
		for (int i = 0; i < MATRIX_SIZE; ++i) {
			elements[i] = new int[MATRIX_SIZE];
			for (int j = 0; j < MATRIX_SIZE; ++j) {
				elements[i][j] = 0;
			}
		}
	}

	//Инициализация матрицы со случайными числами
	void fill_random() {
		elements = new int* [MATRIX_SIZE];
		for (int i = 0; i < MATRIX_SIZE; ++i) {
			elements[i] = new int[MATRIX_SIZE];
			for (int j = 0; j < MATRIX_SIZE; ++j) {
				elements[i][j] = rand() % (e - s + 1) + s;
			}
		}
	}
	void print() {
		cout << endl;
		for (int i = 0; i < MATRIX_SIZE; ++i) {
			cout << "|\n";
			for (int j = 0; j < MATRIX_SIZE; ++j) {
				cout << elements[i][j] << "\t";
			}
			cout << "|" << endl;
		}
	}
};

//Объявление функций
void multiply(Matrix& r, const Matrix& m1, const Matrix& m2);
void one_thread(Matrix& r, long long& elapsed_time, const Matrix& m1, const Matrix& m2);
void multiple_threads(Matrix& r, long long& elapsed_time, const Matrix& m1, const Matrix& m2);
void multiply_with_threads(Matrix& result, const int thread_number, const Matrix& m1, const Matrix& m2);
void timer(void(*execution_function)(Matrix& r, long long& elapsed_time, const Matrix& m1, const Matrix& m2));
long long milliseconds_now();

int main() {
	srand(time(0));
	//Вызов комплексной функции измерения среднего времени проведения умножения матриц в одном потоке
	cout << "Calculating time with one thread..." << endl;
	timer(one_thread);
	//Вызов комплексной функции измерения среднего времени проведения умножения матриц в некотором количестве потоков
	cout << "Calculating time with " << THREADS_NUMBER << "threads..." << endl;
	timer(multiple_threads);
	cout << "Calculation finished.";
}

//Функция для измерения среднего времени, необходимого для выполнения переданной операции
void timer(void(*execution_function)(Matrix& r, long long& elapsed_time, const Matrix& m1, const Matrix& m2)) {

	Matrix m1, m2, r; //Инициализируем матрицы, для работы следующих функций
	m1.fill_random();
	m2.fill_random();//Заполняем первую и вторую матрицы случайными числами
	r.fill_zeros();//Инициализируем матрицу результатов с 0

	long long total_time = 0.0; //Инициализация общего времени
	for (int i = 0; i < N_EXECUTIONS; ++i) {
		long long elapsed_time = 0.0; //Инициализация прошедшего времени
		execution_function(r, elapsed_time, m1, m2); //Выполняем переданную по ссылке функцию
		total_time += elapsed_time; //Подсчитываем общее время выполнения 
	}
	cout << "\nAverage execution took\n" << (double)total_time / N_EXECUTIONS << " ms" << endl; //Подсчёт среднего времени выполнения
}

//Функция перемножения двух матриц в одном потоке
void multiply(Matrix& r, const Matrix& m1, const Matrix& m2) {
	for (int i = 0; i < MATRIX_SIZE; ++i) {
		for (int j = 0; j < MATRIX_SIZE; ++j) {
			int result = 0;
			for (int k = 0; k < MATRIX_SIZE; ++k) {
				const int e1 = m1.elements[i][k];
				const int e2 = m2.elements[k][j];
				result += e1 * e2;
			}
			r.elements[i][j] = result; //Передача результата ячейке матрицы с результатами
		}
	}
	//r.print(); //Проверка работы функции умножения
}

//Функция выполнения одной операции перемножения матриц в одном потоке и замера времени
void one_thread(Matrix& r, long long& elapsed_time, const Matrix& m1, const Matrix& m2) {
	cout << "Starting single thread execution..." << endl;
	long long start_time = milliseconds_now(); //Получаем время начала операции

	cout << "Calculating...." << endl;
	multiply(r, m1, m2); //Выполняем умножение

	long long end_time = milliseconds_now(); //Получаем время окончания операции
	cout << "Finishing single thread execution..." << endl;

	elapsed_time = end_time - start_time; //Получаем затраченное на операцию время
}

//Функция для распределения операции умножения на несколько потоков
void multiply_witht_threads(Matrix& result, const int thread_number, const Matrix& m1, const Matrix& m2) {

	const int n_elements = (MATRIX_SIZE * MATRIX_SIZE);  //Находим кол-во элементов в одной матрице
	const int n_operations = n_elements / THREADS_NUMBER; //Находим количество операций, которые мы можем провести в отдельных потоках
	//const int rest_operations = n_elements % THREADS_NUMBER; //Остаток операций при нечётном количестве

	int start_op, end_op; //Начальная и конечная операции

	if (thread_number == 0) { //Первый поток, вычисление количества операций в нём
		start_op = n_operations * thread_number;
		end_op = (n_operations * (thread_number + 1)); //+ rest_operations;
	}
	else { //Вычисление операций для остальных потоков
		start_op = n_operations * thread_number;// +rest_operations;
		end_op = (n_operations * (thread_number + 1));// +rest_operations;
	}
	for (int op = start_op; op < end_op; ++op) { //Выполнение умножения 
		const int row = op % MATRIX_SIZE; //Находим нужную нам строку
		const int col = op / MATRIX_SIZE; //Находим нужную нам колонку
		//В данном алгоритме мы будем двигаться чаще по строкам, так как по колонкам
		//мы будем сдвигаться только при окончании 1000 (MATRIX_SIZE) операций умножения.
		// Данные значения нужны для прохождения по обоим массивам до конца
		int r = 0; //Инициализация результата
		for (int i = 0; i < MATRIX_SIZE; ++i) { //Проходим по всей длине строки/столбца
			const int e1 = m1.elements[row][i]; //Берём элемент из строки
			const int e2 = m2.elements[i][col]; //Берём элемент из столбца
			r += e1 * e2; //Добавляем результат их перемножения к результату
		}
		m.lock(); //Предотвращаем одновременный доступ
		result.elements[row][col] = r; //Передаём результат в матрицу
		m.unlock(); //Открываем доступ к результатам другим потокам
	}
}

//Функция для подсчёта времени выполнения операций в нескольких потоках
void multiple_threads(Matrix& r, long long& elapsed_time, const Matrix& m1, const Matrix& m2) {
	cout << "Starting multithreading execution..." << endl;
	long long start_time = milliseconds_now(); //Время начала
	//Запускаем нужное количество потоков
	thread thread1(multiply_witht_threads, ref(r), 0, ref(m1), ref(m2));
	thread thread2(multiply_witht_threads, ref(r), 1, ref(m1), ref(m2));
	thread thread3(multiply_witht_threads, ref(r), 2, ref(m1), ref(m2));
	thread thread4(multiply_witht_threads, ref(r), 3, ref(m1), ref(m2));
	thread thread5(multiply_witht_threads, ref(r), 4, ref(m1), ref(m2));
	//И затем ждём их завершения
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	thread5.join();
	//Получаем время окончания операций
	long long end_time = milliseconds_now();
	elapsed_time = end_time - start_time; //Вычисляем прошедшее время
	cout << "Finishing multithreading execution... Time: " << elapsed_time << endl;
}

//Функция для получения точного времени
long long milliseconds_now() {
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / s_frequency.QuadPart;
	}
	else {
		return GetTickCount();
	}
}