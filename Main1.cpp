#include <iostream>
#include <omp.h>
#include <climits>

using namespace std;

const int HEIGHT = 10000;
const int WIDTH = 10000;

int dataGrid[HEIGHT][WIDTH];

void fillArray() {
    #pragma omp parallel for
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            dataGrid[row][col] = row + col;
        }
    }
    // Встановимо унікальне значення для перевірки мінімального рядка
    dataGrid[HEIGHT / 2][0] = -10000;
}

long long computeTotal() {
    long long grandTotal = 0;
    double begin = omp_get_wtime();

    #pragma omp parallel for reduction(+:grandTotal)
    for (int r = 0; r < HEIGHT; r++) {
        for (int c = 0; c < WIDTH; c++) {
            grandTotal += dataGrid[r][c];
        }
    }

    double end = omp_get_wtime();
    cout << "[INFO] Array sum completed in " << end - begin << " sec." << endl;
    return grandTotal;
}

void detectMinRow(int& targetRow, long long& minRowSum) {
    minRowSum = LLONG_MAX;
    targetRow = -1;
    double start = omp_get_wtime();

    #pragma omp parallel
    {
        int localRow = -1;
        long long localMin = LLONG_MAX;

        #pragma omp for
        for (int r = 0; r < HEIGHT; r++) {
            long long rowAggregate = 0;
            for (int c = 0; c < WIDTH; c++) {
                rowAggregate += dataGrid[r][c];
            }

            if (rowAggregate < localMin) {
                localMin = rowAggregate;
                localRow = r;
            }
        }

        #pragma omp critical
        {
            if (localMin < minRowSum) {
                minRowSum = localMin;
                targetRow = localRow;
            }
        }
    }

    double stop = omp_get_wtime();
    cout << "[INFO] Minimum row located in " << stop - start << " sec." << endl;
}

int main() {
    fillArray();

    int lowestRow;
    long long smallestRowSum;
    long long fullSum;

    double globalStart = omp_get_wtime();

    #pragma omp parallel sections
    {
        #pragma omp sectiongo
        {
            fullSum = computeTotal();
        }

        #pragma omp section
        {
            detectMinRow(lowestRow, smallestRowSum);
        }
    }

    double globalEnd = omp_get_wtime();

    cout << ">> Total value of all elements: " << fullSum << endl;
    cout << ">> Row with lowest total: #" << lowestRow << " (sum = " << smallestRowSum << ")" << endl;
    cout << "[INFO] Combined runtime: " << globalEnd - globalStart << " sec." << endl;

    return 0;
}
