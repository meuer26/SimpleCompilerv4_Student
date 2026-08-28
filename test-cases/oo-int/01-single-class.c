class Animal
{
public:
    virtual int speak(int n) { return n + 1; }
};

extern "C" int func(int argA)
{
    Animal a;
    int x = a.speak(argA);
    return x;
}
