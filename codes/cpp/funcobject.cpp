#include <vector>
#include <string>
#include <tr1/memory>
#include <tr1/functional>

using namespace std;
class GameCharacter;

int DefaultHealthCalc(const GameCharacter &gc)
{
    return 0;
}

class GameCharacter
{
public:
    typedef std::tr1::function<int (const GameCharacter &)> HealthCalcFunc;

    explicit GameCharacter(HealthCalcFunc fun = DefaultHealthCalc)
	:f_health(fun)
    {}

private:
    HealthCalcFunc f_health;
};

int main(int argc, char *argv[])
{
    

    return 0;
}
