#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>

template <typename RandomIt>
int parallel_sum(RandomIt beg, RandomIt end) {
	auto len = end - beg;
	if (len < 1000)
		return std::accumulate(beg, end, 0);

	RandomIt mid = beg + len / 2;
	auto handle = std::async(std::launch::async,
		parallel_sum<RandomIt>, mid, end);
	int sum = parallel_sum(beg, mid);
	return sum + handle.get();
}

void test1() {
	std::cout << "Hello world! id=" << std::this_thread::get_id() << std::endl;

	std::vector<int> v(12000, 1);
	std::cout << "The sum is " << parallel_sum(v.begin(), v.end()) << '\n';

	std::vector<std::future<void> > futures;
	for (int i = 0; i < 10; i++) {
		auto fut = std::async([]() {
			std::cout << std::this_thread::get_id() << "\n";
			});
		futures.push_back(std::move(fut));
	}
	std::for_each(futures.begin(), futures.end(), [](std::future<void>& fut) { fut.wait(); });
	std::this_thread::sleep_for(std::chrono::seconds(2));
}

class result_monitor {
public:
	void add(std::string value) {
		std::lock_guard<std::mutex> lock(_mutex);
		_result.push_back(value);
	}
	void print() {
		std::for_each(_result.begin(), _result.end(), [](std::string& v) { std::cout << v << std::endl; });
	}
private:
	std::mutex _mutex;
	std::vector<std::string> _result;
};

void my_thread(int index, result_monitor& monitor) {
	for (int j = 0; j < 10; j++) {
		char s[100];
		sprintf(s, "th #%d - %2.2d", index, j);
		monitor.add(s);
	}
}

int main() {
	result_monitor monitor;
	std::vector<std::future<void> > futures;

	for (int i = 0; i < 5; i++) {
		auto fut = std::async(&my_thread, i + 1, std::ref(monitor));
		futures.push_back(std::move(fut));
	}
	std::for_each(futures.begin(), futures.end(), [](std::future<void>& fut) { fut.wait(); });

	monitor.print();

	return 0;
}