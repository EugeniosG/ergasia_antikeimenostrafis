#include <iostream>
#include <cstring>

using namespace std;

void print_help(){

}

int main(int argc, char* argv[]){
    if (argv[1] == "--help") {
        print_help();
    }
    return 0;
}