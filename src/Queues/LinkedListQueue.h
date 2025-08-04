#pragma once
#include <IQueue.h>
#include <atomic>

template<typename T>
class LinkedListQueue : public IQueue<T>{
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        Node(T data) : data(data), next(nullptr) {}
    };
    std::atomic<Node*> head;
    std::atomic<Node*> tail;

public:

    LinkedListQueue() {
        Node* dummy = new Node(nullptr);
        head.store(dummy);
        tail.store(dummy);
    }
    ~LinkedListQueue();

    void Enqueue(const T& value) override {
        Node* new_node = new Node(value);
        while (true) {
            Node* last = tail.load();
            Node* next = last->next.load();
            if (last == tail.load()) {
                if (next == nullptr) {
                    //CAS: compare and swap = compare exchange weak
                    if (last->next.compare_exchange_weak(next, new_node)) {
                        tail.compare_exchange_weak(last, new_node);
                        return;
                    }
                } else {
                    tail.compare_exchange_weak(last, next);
                }
            }
        }
        //CAS here?
    }

    bool Dequeue(T& out) override {
        while (true) {
            Node* first = head.load();
            Node* last = tail.load();
            Node* next = first->next.load();
            if (first == head.load()) {
                if (first == last) {
                    if (next == nullptr) {
                        return false;
                    }
                    tail.compare_exchange_weak(last, next);
                } else {
                    out = next-> data;
                    if (head.compare_exchange_weak(first, next)) {
                        delete first; //free old dummy node
                        return true;
                    }
                }
            }
        }
    }

};
#ifndef CLIONPROJECTS_LINKEDLISTQUEUE_H
#define CLIONPROJECTS_LINKEDLISTQUEUE_H

#endif //CLIONPROJECTS_LINKEDLISTQUEUE_H
