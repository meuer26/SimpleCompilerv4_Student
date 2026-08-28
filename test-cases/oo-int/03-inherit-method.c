class Animal
{
public:
    virtual int speak(int n) { return n + 1; }
    virtual int eat(int n) { return n - 1; }
};

class Dog : public Animal
{
public:
    virtual int speak(int n) { return n * 2; }
};

extern "C" int func(int argA)
{
    Dog d;
    int y = d.eat(argA);
    return y;
}
