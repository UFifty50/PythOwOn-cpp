#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP


#include <memory>
#include <optional>
#include <utility>


namespace LinkedList {
    template <typename T>
    class Single {
        struct Node {
            T data;
            std::unique_ptr<Node> next;

            Node(T data) : data(std::move(data)), next(nullptr) {}
        };

    public:
        Single() : head(nullptr), tail(nullptr) {}

        void push(T data) {
            std::unique_ptr<Node> node = std::make_unique<Node>(std::move(data));
            if (head == nullptr) {
                head = std::move(node);
                tail = head.get();
            } else {
                tail->next = std::move(node);
                tail = tail->next.get();
            }
        }

        std::optional<T> pop() {
            if (head == nullptr) return std::nullopt;

            T data = std::move(head->data);
            head = std::move(head->next);
            return data;
        }

        bool isEmpty() { return head == nullptr; }

        Node search(T data) {
            Node* node = head.get();
            while (node != nullptr) {
                if (node->data == data) return *node;
                node = node->next.get();
            }
            return nullptr;
        }

        void remove(T data) {
            if (head == nullptr) return;

            if (head->data == data) {
                head = std::move(head->next);
                return;
            }

            Node* node = head.get();
            while (node->next != nullptr) {
                if (node->next->data == data) {
                    node->next = std::move(node->next->next);
                    return;
                }
                node = node->next.get();
            }
        }

        void clear() {
            head = nullptr;
            tail = nullptr;
        }

    private:
        std::unique_ptr<Node> head;
        Node* tail;
    };
}  // namespace LinkedList

#endif
