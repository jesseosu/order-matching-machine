// order_matching_engine.cpp
// A simple multithreaded order matching engine simulation in C++

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <chrono>
#include <map>

using namespace std;

struct Order {
    int id;
    string type; // "buy" or "sell"
    int price;
    int quantity;
};

mutex book_mutex;
condition_variable cv;
queue<Order> order_queue;
atomic<bool> running(true);

// Order books: price -> total quantity
map<int, int, greater<int>> buy_book; // Highest price first
map<int, int> sell_book;              // Lowest price first

void match_orders(const Order& new_order) {
    if (new_order.type == "buy") {
        for (auto it = sell_book.begin(); it != sell_book.end() && new_order.price >= it->first;) {
            if (new_order.quantity == 0) break;
            int match_qty = min(new_order.quantity, it->second);
            cout << "Matched BUY: " << match_qty << " @ $" << it->first << endl;
            it->second -= match_qty;
            const_cast<Order&>(new_order).quantity -= match_qty;
            if (it->second == 0) sell_book.erase(it++);
            else ++it;
        }
        if (new_order.quantity > 0) buy_book[new_order.price] += new_order.quantity;
    } else if (new_order.type == "sell") {
        for (auto it = buy_book.begin(); it != buy_book.end() && new_order.price <= it->first;) {
            if (new_order.quantity == 0) break;
            int match_qty = min(new_order.quantity, it->second);
            cout << "Matched SELL: " << match_qty << " @ $" << it->first << endl;
            it->second -= match_qty;
            const_cast<Order&>(new_order).quantity -= match_qty;
            if (it->second == 0) buy_book.erase(it++);
            else ++it;
        }
        if (new_order.quantity > 0) sell_book[new_order.price] += new_order.quantity;
    }
}

void matching_thread() {
    while (running) {
        unique_lock<mutex> lock(book_mutex);
        cv.wait(lock, [] { return !order_queue.empty() || !running; });
        while (!order_queue.empty()) {
            Order ord = order_queue.front();
            order_queue.pop();
            match_orders(ord);
        }
    }
}

void submit_order(int id, const string& type, int price, int quantity) {
    lock_guard<mutex> lock(book_mutex);
    order_queue.push(Order{id, type, price, quantity});
    cv.notify_one();
}

void simulate_orders() {
    this_thread::sleep_for(chrono::milliseconds(100));
    submit_order(1, "buy", 101, 50);
    submit_order(2, "sell", 100, 30);
    submit_order(3, "sell", 99, 30);
    submit_order(4, "buy", 102, 60);
    submit_order(5, "sell", 101, 50);
    submit_order(6, "buy", 100, 40);
    this_thread::sleep_for(chrono::milliseconds(500));
}

int main() {
    thread matcher(matching_thread);
    thread submitter(simulate_orders);

    submitter.join();
    this_thread::sleep_for(chrono::milliseconds(1000));
    running = false;
    cv.notify_all();
    matcher.join();

    cout << "\nRemaining Buy Book:\n";
    for (const auto& [price, qty] : buy_book) {
        cout << "BUY: " << qty << " @ $" << price << endl;
    }

    cout << "\nRemaining Sell Book:\n";
    for (const auto& [price, qty] : sell_book) {
        cout << "SELL: " << qty << " @ $" << price << endl;
    }
    return 0;
}
