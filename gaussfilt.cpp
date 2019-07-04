#include <cmath>
#include <iomanip>
#include <iostream>
#include <iostream>
using namespace std;

void FilterCreation(double GKernel[][5])
{
    double sigma = 1.0;
    double r, s = 2.0 * sigma * sigma;

    double sum = 0.0;

    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            r = sqrt(x * x + y * y);
            GKernel[x + 2][y + 2] = (exp(-(r * r) / s)) / (M_PI * s);
            sum += GKernel[x + 2][y + 2];
        }
    }

    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            GKernel[i][j] /= sum;
}

int main()
{
    double GKernel[5][5];
    FilterCreation(GKernel);

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j)
            cout << GKernel[i][j] << "\t";
        cout << endl;
    }
}
