class Super {

};

class Hello : public Super {
public:
    Hello() {
        x = 0;
    }

    void print()
    {
        x = 5;
    }

    void hi(int a, int b)
    {
        int c;
        c = a + b;
    }

private:
    int x;
};

void aaaaa()
{
    int a = 5;
    a = a + 2;
}

void fuck_yes()
{
    int a = 5;
    a = a + 2;
}

int sum(int a)
{
    return a;
}

int main()
{
    int a = 3;
    int b = 5;
    int c = sum(a);
    return c;
}