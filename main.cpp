#include <iostream>
#include "SkipList.h"

int main() {
    SkipList<int, std::string> skiplist(32);
    skiplist.insert_element(1, "一");
    skiplist.insert_element(2, "二");
    skiplist.insert_element(3, "三");
    skiplist.insert_element(4, "四");
    skiplist.insert_element(5, "五");
    skiplist.insert_element(10, "十");
    skiplist.insert_element(8, "八");
    skiplist.insert_element(20, "二十");

    std::cout << "skipList size:" << skiplist.size() << std::endl;

    skiplist.query_element(10);
    skiplist.query_element(1);
    skiplist.query_element(18);

    skiplist.dump_file();

    skiplist.display_list();

    skiplist.delete_element(3);
    skiplist.delete_element(7);
    skiplist.delete_element(2);

    std::cout << "skipList size:" << skiplist.size() << std::endl;

    skiplist.display_list();

    return 0;
}
