#pragma once
#include <functional>
#include <memory>

class MyString
{
	enum {DEFAULT_SIZE = 1024};
public:
	MyString();
	MyString(const MyString & s);
	~MyString();
	MyString & operator = (const MyString & s);
private:
	char * buf;
};

template <typename Func, class ... args>
auto fun_wapper(Func &&f, args && ...params) -> decltype(f(std::forward<args>(params)...))
{
	return f(std::forward<args>(params)...);
}

//variable template function
template <typename T>
void test_template(T t)
{
	std::cout << t << std::endl;
}

//template <class T, typename ...Args>
//void test_template(T head, Args...args)
//{
//	std::cout << head << std::endl;
//	test_template(args...);
//}

template <std::size_t I = 0, typename Tuple>
typename std::enable_if<I == std::tuple_size<Tuple>::value >::type printtp(Tuple t)
{

}

template <std::size_t I = 0, typename Tuple>
typename std::enable_if<I < std::tuple_size<Tuple>::value >::type printtp(Tuple t)
{
	std::cout << std::get<I>(t) << std::endl;
	printtp<I + 1>(t);
}

template <typename ...T>
void test_template1(T ...args)
{
	printtp(std::make_tuple(args...));
}

template <class ... args>
void test_template2(args...params)
{
	std::initializer_list<int>{([&] {std::cout << params << std::endl; }(), 0) ...};
	//std::initializer_list<int>{(test_template(params), 0) ...};
}

//variable template class
template <class ...args> struct sum;

template <class first, class ...rest>
struct sum<first, rest...>
{
	enum { value = sum<first>::value + sum<rest...>::value; };
};

template <class last>
struct sum<last>
{
	enum { value = sizeof(last); };
};

template <class T>
class ObjectPool
{
	template <typename ... args>
	using Constructor = std::function<std::shared_ptr<T>(args...)>;
};

template <typename T>
struct Constructor
{
	//typedef typename std::remove_reference<T>::type U;
	using U = typename std::remove_reference<T>::type;
	Constructor():m_ptr(new U) {}

	typename std::add_lvalue_reference<T>::type
		Get()
	{
		return *m_ptr.get();
	}

private:
	std::unique_ptr<T> m_ptr;
};