#include <vector>
#include <algorithm>
#include <execution>
#include <iostream>
#include <chrono>
#include <random>
#include <future>
#include <thread>
using namespace std;

// Версия 1: с std::execution::par
void EvenOddSort_Par(std::vector<int>& arr) {
    std::vector<int> evens, odds;

    for (int x : arr) {
        if (x % 2 == 0)
            evens.push_back(x);
        else
            odds.push_back(x);
    }

    std::sort(std::execution::par, evens.begin(), evens.end());
    std::sort(std::execution::par, odds.begin(), odds.end(), std::greater<int>());

    auto it = arr.begin();

    for (int x : evens) {
        *it = x;
        ++it;
    }

    for (int x : odds) {
        *it = x;
        ++it;
    }
}

// Версия 2: с обычным std::sort
void EvenOddSort_Seq(std::vector<int>& arr) {
    std::vector<int> evens, odds;

    for (int x : arr) {
        if (x % 2 == 0)
            evens.push_back(x);
        else
            odds.push_back(x);
    }

    std::sort(evens.begin(), evens.end());
    std::sort(odds.begin(), odds.end(), std::greater<int>());

    auto it = arr.begin();

    for (int x : evens) {
        *it = x;
        ++it;
    }

    for (int x : odds) {
        *it = x;
        ++it;
    }
}

// Версия 3: std::execution::par (обычный массив)
void EvenOddSort_Array(int* arr, int n) {
    int* evens = new int[n];
    int* odds = new int[n];
    int evens_count = 0;
    int odds_count = 0;

    for (int i = 0; i < n; ++i) {
        if (arr[i] % 2 == 0) {
            evens[evens_count++] = arr[i];
        }
        else {
            odds[odds_count++] = arr[i];
        }
    }

    std::sort(std::execution::par, evens, evens + evens_count);
    std::sort(std::execution::par, odds, odds + odds_count, std::greater<int>());

    int idx = 0;
    for (int i = 0; i < evens_count; ++i) {
        arr[idx++] = evens[i];
    }
    for (int i = 0; i < odds_count; ++i) {
        arr[idx++] = odds[i];
    }

    delete[] evens;
    delete[] odds;
}

// Кастомный parallel quicksort (рекурсивный, с std::async)
void quicksort(int* arr, int low, int high) {
    if (low < high) {
        int pivot = arr[high];
        int i = low - 1;

        for (int j = low; j < high; ++j) {
            if (arr[j] <= pivot) {
                ++i;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[high]);
        int pi = i + 1;

        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

// Параллельная версия quicksort
void parallel_quicksort(int* arr, int low, int high) {
    if (low < high) {
        int pivot = arr[high];
        int i = low - 1;

        for (int j = low; j < high; ++j) {
            if (arr[j] <= pivot) {
                ++i;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[high]);
        int pi = i + 1;

        if (pi - low > 10000) { // Если подмассив большой — запускаем параллельно
            auto left = std::async(std::launch::async, parallel_quicksort, arr, low, pi - 1);
            parallel_quicksort(arr, pi + 1, high);
            left.wait();
        }
        else {
            quicksort(arr, low, pi - 1);
            quicksort(arr, pi + 1, high);
        }
    }
}

// Версия 4: с кастомным parallel_quicksort
void EvenOddSort_Quicksort(int* arr, int n) {
    int* evens = new int[n];
    int* odds = new int[n];
    int evens_count = 0;
    int odds_count = 0;

    for (int i = 0; i < n; ++i) {
        if (arr[i] % 2 == 0) {
            evens[evens_count++] = arr[i];
        }
        else {
            odds[odds_count++] = arr[i];
        }
    }

    parallel_quicksort(evens, 0, evens_count - 1);
    parallel_quicksort(odds, 0, odds_count - 1);

    // Разворачиваем нечетные (по убыванию)
    std::reverse(odds, odds + odds_count);

    int idx = 0;
    for (int i = 0; i < evens_count; ++i) {
        arr[idx++] = evens[i];
    }
    for (int i = 0; i < odds_count; ++i) {
        arr[idx++] = odds[i];
    }

    delete[] evens;
    delete[] odds;
}

// Оптимизированный counting_sort (работает на массиве)
void counting_sort(int* arr, int n, int exp, int* output) {
    int count[10] = { 0 };

    for (int i = 0; i < n; ++i)
        count[(arr[i] / exp) % 10]++;

    for (int i = 1; i < 10; ++i)
        count[i] += count[i - 1];

    for (int i = n - 1; i >= 0; --i) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }

    for (int i = 0; i < n; ++i)
        arr[i] = output[i];
}

// Radix Sort для массива
void radix_sort(int* arr, int n) {
    if (n <= 1) return;

    int max_val = arr[0];
    for (int i = 1; i < n; ++i) {
        if (arr[i] > max_val) max_val = arr[i];
    }

    int* temp = new int[n];

    for (int exp = 1; max_val / exp > 0; exp *= 10) {
        counting_sort(arr, n, exp, temp);
    }

    delete[] temp;
}

// Версия 5: radix sort (на массиве)
void EvenOddSort_Radix_Array(int* arr, int n) {
    int* evens = new int[n];
    int* odds = new int[n];
    int evens_count = 0;
    int odds_count = 0;

    for (int i = 0; i < n; ++i) {
        if (arr[i] % 2 == 0) {
            evens[evens_count++] = arr[i];
        }
        else {
            odds[odds_count++] = arr[i];
        }
    }

    radix_sort(evens, evens_count);
    radix_sort(odds, odds_count);

    // Разворот нечетных (для убывания)
    std::reverse(odds, odds + odds_count);

    int idx = 0;
    for (int i = 0; i < evens_count; ++i) {
        arr[idx++] = evens[i];
    }
    for (int i = 0; i < odds_count; ++i) {
        arr[idx++] = odds[i];
    }

    delete[] evens;
    delete[] odds;
}

double measure_average_time(std::vector<int> original, void (*func)(std::vector<int>&), int runs) {
    long long total_duration_us = 0;

    for (int i = 0; i < runs; ++i) {
        auto arr = original;

        auto start = std::chrono::high_resolution_clock::now();
        func(arr);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        total_duration_us += duration.count();
    }

    return static_cast<double>(total_duration_us) / runs / 1000.0;  // в миллисекундах
}

double measure_average_time_array(const int* original, int n, void (*func)(int*, int), int runs) {
    long long total_duration_us = 0;

    for (int i = 0; i < runs; ++i) {
        int* arr = new int[n];
        std::copy(original, original + n, arr);

        auto start = std::chrono::high_resolution_clock::now();
        func(arr, n);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        total_duration_us += duration.count();

        delete[] arr;
    }

    return static_cast<double>(total_duration_us) / runs / 1000.0;
}

int main() {
    setlocale(LC_ALL, "RU");
    const int N = 1'000'000;
    const int RUNS = 50;

    // Генерация случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 1000000);

    std::vector<int> original_vec(N);
    for (int i = 0; i < N; ++i) {
        original_vec[i] = dis(gen);
    }

    // Конвертируем в обычный массив
    int* original_arr = new int[N];
    std::copy(original_vec.begin(), original_vec.end(), original_arr);

    // Измерение std::execution::par
    std::cout << "Измерение std::execution::par..." << std::endl;
    double avg_time_par = measure_average_time(original_vec, EvenOddSort_Par, RUNS);
    std::cout << "Среднее время (std::execution::par): " << avg_time_par << " ms" << std::endl;

    // Измерение std::sort (обычная)
    std::cout << "Измерение std::sort (без параллелизма)..." << std::endl;
    double avg_time_seq = measure_average_time(original_vec, EvenOddSort_Seq, RUNS);
    std::cout << "Среднее время (std::sort): " << avg_time_seq << " ms" << std::endl;

    // std::execution::par (обычный массив)
    std::cout << "Измерение std::execution::par (array)..." << std::endl;
    double avg_time_arr = measure_average_time_array(original_arr, N, EvenOddSort_Array, RUNS);
    std::cout << "Среднее время (std::execution::par, array): " << avg_time_arr << " ms" << std::endl;

    // обычный массив с parallel quicksort
    std::cout << "Измерение parallel quicksort..." << std::endl;
    double avg_time_quick = measure_average_time_array(original_arr, N, EvenOddSort_Quicksort, RUNS);
    std::cout << "Среднее время parallel quicksort(array): " << avg_time_quick << " ms" << std::endl;

    std::cout << "Измерение radix sort (array)..." << std::endl;
    double avg_time_radix_arr = measure_average_time_array(original_arr, N, EvenOddSort_Radix_Array, RUNS);
    std::cout << "Среднее время (radix sort, array): " << avg_time_radix_arr << " ms" << std::endl;

    return 0;
}