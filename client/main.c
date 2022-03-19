#include <dispatcher.h>
#include <check_errors.h>

int main(int argc, char const *argv[])
{
    int dis = dispatcher(argc, (char**) argv);
    CHECK_OPERATION(dis == -1, return -1);

    return 0;
}
