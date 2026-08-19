#include <iostream>
#include <cstring>
using namespace std;
int main() {
    char buffer[64];
    cout << "Enter your name: ";
    gets(buffer);
    cout << "Hello " << buffer << "!\n";
    return 0;
}
