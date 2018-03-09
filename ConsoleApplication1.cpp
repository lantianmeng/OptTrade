// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <map>
#include <functional>
#include <memory>

#include <thread>
#include <mutex>
#include <future>

#include "tips.h"
#include "threadpool.h"

class A
{
public :
	static int Get()
	{
		std::cout << "A::Get" << std::endl;
		return i;
	}

	static int Plus(int x)
	{
		std::cout << "A::Plus" << std::endl;
		return i + x;
	}
private:
	static const auto i = 0;
};

struct Base
{
	static const char* Get()
	{
		return "Base";
	}
	void fun() { std::cout << "Base" << std::endl; }
};

struct Derived :public Base
{
	//using Base::fun;
	void fun(int a) { std::cout << "Derived" << std::endl; }

	void print(int a, int b) { std::cout << "Derived " << a  << " " << b << std::endl; }

	int add(int i, int j)
	{
		std::cout << "Derived  add()" << std::endl;
		return i + j;
	}

	int add_p(int i, int j, std::promise<double> & pr) 
	{
		std::cout << "Derived  promise add()" << std::endl;
		pr.set_value(11.5);
		return i + j;
	}
};

template<class T>
auto Func() -> decltype(T::Get())
{
	return T::Get();
}

template<typename T>
using func = void(*)(T, T);

template<typename T>
struct func1
{
	typedef void(*type)(T, T);
};

void add(int i, int j)
{
	std::cout << i + j << std::endl;
}

int plus(int i, int j)
{
	std::cout << i + j << std::endl;
	return  i + j;
}

void print_thread()
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "another thread" << std::endl;
}

struct Job
{
	void operator() (const std::string & name )
	{
		std::cout << "Job:" << name.c_str()  << std::endl;
	}
};

int main()
{
	std::map<int, std::string> test;
	test.emplace(2, "4");
	test.emplace(1, "3");

	//Derived xx;
	//xx.fun();
	//auto tt = test.find(2);
	//if (tt != test.end())
	for (auto tt : test)
	{
		//std::cout << tt->first  << "\t" << (tt->second).c_str();
		std::cout << tt.first << "\t" << (tt.second).c_str() << std::endl;
	}

	std::cout << Func<A>() << "\t" << Func<Base>() << std::endl;

	func<int> callback = add;
	callback(1, 3);

	//using func_t = int(*)(int);
	func<int> f = [](int i, int j) { std::cout << i + j << std::endl; };
	f(12, 13) ;

	std::function<void(int, int)> ff = f;
	ff(2, 13);

	std::function<void(double, double)> dd = [](double i, double j) { std::cout << i + j << std::endl; };
	dd(1.5, 2.1);

	Derived dd_obj;
	//std::function<void(int, int)> fr = std::bind(add, std::placeholders::_1, std::placeholders::_2);
	std::function<void(int, int)> fr = std::bind(&Derived::print, &dd_obj, std::placeholders::_1, std::placeholders::_2);
	fr(3, 5);
	//auto frxx = std::bind(add, std::placeholders::_1, std::placeholders::_2);
	//frxx(3, 5);

	auto f2 = std::bind(&Derived::print, &dd_obj, std::placeholders::_1, 6);
	f2(2);

	int b = 6;
	int && t = std::move(b);

	//test_template(1, 2, 4, 3);
	//fun_wapper(test_template);
	//fun_wapper(test_template, 1, 2, 4, 3);

	fun_wapper(A::Get);
	fun_wapper(fr, 13, 15);

	//test_template1(1, 2, 4, 3);
	test_template2(11, 22, 44, 33);

	//std::unique_ptr<int> gg(new int);
	std::shared_ptr<int> gg(new int);
	*gg = b;

	//std::unique_ptr<int> hh = gg;
	std::shared_ptr<int> hh = gg;

	std::weak_ptr<int> gh(hh);

	std::mutex lock;
	std::lock_guard<std::mutex> locker(lock);

	using IC = std::integral_constant<int, 111>;
	std::cout << IC::value << " " << typeid(IC::value_type).name() << " " << typeid(IC::type).name() << std::endl;

	Constructor<int> xt;
	xt.Get() = b;
	//std::cout << ix << std::endl;

	
	//std::thread td(print_thread);
	std::thread td(std::mem_fn(&Derived::print), dd_obj, 11, 12);//成员函数做线程参数
	td.detach();

	std::mutex mm;
	std::lock_guard<std::mutex> l_mm(mm);

	std::thread td1(add, 4, 5);
	td1.join();
	std::cout << "test thread block" << std::endl;

	std::packaged_task<int(int, int)>  task(std::bind(&Derived::add, &dd_obj, std::placeholders::_1, std::placeholders::_2));
	std::thread td2(std::ref(task), 44, 55);
	std::future<int> ret = task.get_future();
	std::cout << "packaged_task:" << ret.get() << std::endl;
	td2.join();

	std::promise<double> pr;
	std::thread td3(std::mem_fn(&Derived::add_p), dd_obj, 45, 55, std::ref(pr));
	std::future<double> ret1 = pr.get_future();
	std::cout << "promise:" << ret1.get() << std::endl;
	td3.join();

	std::future<int> ret2 = std::async(std::launch::async, std::mem_fn(&Derived::add), dd_obj, 33, 44);
	std::cout << "async:" << ret2.get() << std::endl;

	threadpool executor(12);

	//std::future<int> fff = executor.commit(plus, 24, 35);
	std::future<int> fff = executor.commit(A::Plus, 13);
	std::cout << "result is " << fff.get() << '\n';

	//executor.AddTask(std::bind(A::Plus, 13));
	executor.commit(A::Get);
	//executor.AddTask(A::Get);
	executor.commit(add, 2, 3);
	//executor.AddTask(std::bind(add, 2, 3));
	executor.commit(Job(), "commit");
	//executor.AddTask(std::bind(Job(), "addtask"));

	executor.commit(fr, 11, 24);
	//executor.AddTask(std::bind(fr, 13, 23)); // commit版本 比 AddTask(需要使用std::bind)版本好用， 但是获取返回值时程序崩溃

	Base *pb = NULL;
	pb->fun();

	system("pause");
    return 0;
}


