#include <iostream>

using namespace std;

class A 
{
    protected:
        int y, x;

    public:
        A(int x, int y) : x(x), y(y) {}

        virtual void print() {
            cout << "A: x=" << this->x << endl;
            cout << "A: y=" << this->y << endl;
        }

        virtual ~A() {
            cout << "Desc A: " << this->x << ", " << this-> y << endl;
        }
};

class B : public A{
    public:
        B(int x, int y) : A(x, y) {}

        void print() {
            cout << "B: x=" << this->x << endl;
            cout << "B: y=" << this->y << endl;
        }

        ~B() {
            cout << "Desc B: " << this->x << ", " << this-> y << endl;
        }
};

int main() 
{
    A* ap = new B(69, 420);

    ap->print();

    delete ap;
}