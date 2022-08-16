#ifndef SKIPLIST
#define SKIPLIST

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <cmath>
#include <string.h>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;
std::string delimiter = ":";

template<typename K, typename V>
class Node {
public:
    Node();
    Node(const K k, const V v, int level);
    ~Node();

    K get_key() const;
    V get_value() const;

    void set_value(V);

    // 指向各个层的下一个节点
    Node<K, V> **forward;
    int node_level;

private:
    K key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level){
    this->key = k;
    this->value = v;
    this->node_level = level;

    this->forward = new Node<K, V>*[level + 1];

    memset(this->forward, 0, sizeof(Node<K, V>*)*(level + 1));
}

template<typename K, typename V>
Node<K, V>::~Node() {
    delete []forward;
}

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
}

template<typename K, typename V>
void Node<K, V>::set_value(V v) {
    this->value = v;
}

template<typename K, typename V>
class SkipList
{
public:
    SkipList(int);
    ~SkipList();

    Node<K, V>* create_node(K, V, int);

    int insert_element(K, V);
    void delete_element(K);
    bool query_element(K);
    void display_list();
    int size();

    int get_random_level();

    void dump_file();
    void load_file();

private:
    bool is_valid_string(const std::string& str);
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);

private:
    // 最大层数
    int _max_level;
    // 当前层数
    int _skip_list_level;
    // 元素数量
    int _element_count;
    // 头节点
    Node<K, V> *_header;

    std::ofstream _file_writer;
    std::ifstream _file_reader;

};

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {
    if (!is_valid_string(str)) return;
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.size());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

template<typename K, typename V>
void SkipList<K, V>::load_file() {
    _file_reader.open(STORE_FILE);
    std::cout << "-----load file-----" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) continue;
        insert_element(*key, *value);
        std::cout << "key: " << *key << ", value: " << *value << std::endl;
    }
    _file_reader.close();
}

template<typename K, typename V>
void SkipList<K, V>::dump_file() {
    std::cout << "-----dump file-----" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];
    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
}

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(K k, V v, int level) {
    Node<K, V>* node = new Node<K, V>(k, v, level);
    return node;
}

template<typename K, typename V>
int SkipList<K, V>::insert_element(K k, V v) {
    mtx.lock();
    Node<K, V>* cur = this->_header;

    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    for (int i = _skip_list_level; i >= 0; i--) {
        while (cur->forward[i] && cur->forward[i]->get_key()  < k) {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }
    cur = cur->forward[0];

    if (cur && cur->get_key() == k) {
        std::cout << "key: " << k << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    if (cur == NULL || cur->get_key() != k) {
        int random_level = get_random_level();

        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level + 1; i < random_level + 1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        Node<K, V>* node = create_node(k, v, random_level);

        for (int i = 0; i <= random_level; i++) {
            node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
        std::cout << "Successfully inserted key: " << k << ", value: " << v << std::endl;
        _element_count++;
    }
    mtx.unlock();
    return 0;
}

template<typename K, typename V>
void SkipList<K, V>::delete_element(K k) {
    mtx.lock();
    Node<K, V>* cur = _header;
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

    for (int i = _skip_list_level; i >= 0; i--) {
        while (cur->forward[i] && cur->forward[i]->get_key() < k) {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }
    cur = cur->forward[0];

    if (cur && cur->get_key() != k) {
        std::cout << "Not Found Key!" << std::endl;
    }
    else if (cur && cur->get_key() == k) {
        for (int i = 0; i < _skip_list_level + 1; i++) {
            if (update[i]->forward[i] != cur) break;

            update[i]->forward[i] = cur->forward[i];
        }

        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level--;
        }

        std::cout << "Successfully Delete key: " << k << std::endl;
        _element_count--;
    }

    mtx.unlock();
    return;
}

template<typename K, typename V>
bool SkipList<K, V>::query_element(K key) {
    std::cout << "------------query_element-----------" << std::endl;
    Node<K, V> *cur = _header;

    // 从高层到低层
    for (int i = _skip_list_level; i >= 0; i--) {
        while (cur->forward[i] && cur->forward[i]->get_key() < key) {
            cur = cur->forward[i];
        }
    }
    cur = cur->forward[0];

    if (cur && cur->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << cur->get_value() << std:: endl;
        return true;
    }

    std::cout << "Not Found Key: " << key << std::endl;
    return false;

}

template<typename K, typename V>
void SkipList<K, V>::display_list() {
    std::cout << "\n----------------Skip List--------------\n";
    for (int i = 0; i <= _skip_list_level; i++) {
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ": " << node->get_value() << "; ";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

template<typename K, typename V>
int SkipList<K, V>::size() {
    return _element_count;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
    int k = 0;
    while (rand() % 2) {
        k++;
    }
    return k < _max_level ? k : _max_level;
}

template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

template<typename K, typename V>
SkipList<K, V>::~SkipList() {
    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    delete _header;
}

#endif
