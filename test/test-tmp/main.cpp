#include <iostream>
using namespace std;
int main()
{
    int *a = new int[10];
    a[11] = 0;
    cout << a[11]<< endl;
    return 0;
}
