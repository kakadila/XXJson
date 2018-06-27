# A tiny JSON library
### No more than 800 lines of code,Although the performance is not very good.
```c++
#include "XXJson.h"
#include <string>
#include <iostream>
using namespace std;

int main()
{
	XXJson json;
	json["name"] = "kaka";
	json["age"] = 19;
	json["is_man"] = true;
	
	XXJson array;
	array.Push(123);
	array.Push("eating");
	array.Push(3.14);
	
	json["array"] = array;
	
	string str = json.ToString();
	
    cout<<str<<endl;
	
	XXJson parseJson;
	parseJson.Parse(str);
	
    cout<< parseJson["name"].GetString() <<endl;
	
	return 0;
}

```
