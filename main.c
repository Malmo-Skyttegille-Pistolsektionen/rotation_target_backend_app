#include "target.h"
#include "app.h"
#include "error_code.h"

#include <unistd.h>

int main()
{
    error_code_t    res;

    res = target_init();
    if (res != ERROR_CODE_OK)
    {
        target_fatal(res);
    }

    res = app_init();
    if (res != ERROR_CODE_OK)
    {
        target_fatal(res);
    }

    res = app_start();
    if (res != ERROR_CODE_OK)
    {
        target_fatal(res);
    }

    while (1)
    {
        sleep(1);
    }

    return 0;
}
