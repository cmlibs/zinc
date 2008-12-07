/* example functions */

#include <iostream>
#include <time.h>
#include "egfunc.h"

double arith(double x, double y, int type)
{
	double ans;
	
	switch(type)
	{
		case 1:
			ans = x+y;
			std::cout << x << " + " << y << " = " << ans << std::endl;
			break;
		case 2:
			ans = x-y;
			std::cout << x << " - " << y << " = " << ans << std::endl;
			break;
		case 3:
			ans = x*y;
			std::cout << x << " * " << y << " = " << ans << std::endl;
			break;
		case 4:
			ans = x/y;
			std::cout << x << " / " << y << " = " << ans << std::endl;
			break;
		default:
			std::cout << "Unknown operation!";
	}
	return ans;
}

int shoe()
{
	int shoes;
	
	std::cout << "number of shoes? ";
	std::cin >> shoes;
	
	if (shoes%2 != 0)
		std::cout << "not in pairs\n";
	else
		std::cout << "thats " << shoes/2 << " pairs\n" << std::endl;
	
	return shoes/2;
}

char *get_time()
{
    time_t ltime;
    time(&ltime);
    return ctime(&ltime);
}

