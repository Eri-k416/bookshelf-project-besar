#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <optional>
using namespace std;

class myObj {
    public:
        int x = 0;


        myObj() = default;

        myObj(int y) {
            x = y;
        };
};

struct myStructure {
    optional<myObj> object;
};

int main() {
    myStructure objectstruct;
    
    cout << objectstruct.object << endl;
};