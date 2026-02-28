//expect 3
//tags: simple, scoping
int main()
{
    int x;
    {
        x = 3;
    }
    {
        return x;
    }
}