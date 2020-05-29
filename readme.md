# 天问之路

此项目用以记录跟随sakura师傅学习的经历，和平时自己学习的点滴。项目不会包括具体的知识，只是一个类似周记的记录，用来给自己看。

---
<details>
    <summary>5.15-5.29</summary>
    - STL
        目标：熟悉c++语法，了解STL标准库的实现。</br>
        进度：allocator-> iterator-> basefunction -> list -> vector->hashtable</br>
        收获：</br>
            - <a herf = "https://www.xuebuyuan.com/1919018.html">traits的使用</a></br>
            - <a herf = "https://blog.csdn.net/qq_25343557/article/details/89110319">自平衡树算法（AVL）</a></br>
            - 红黑树（TODO）
            - vector erase存在的漏洞<br>
                ```
                  iterator erase(iterator position) {
                        if (position + 1 != end())
                        {
                        //把从 position+1 到 finish 之间的元素一个一个复制到从 position 指向
                        //的空间，这样，就把 position 原来指向的元素个覆盖了
                        copy(position + 1, finish, position);
                        }
                        --finish;
                        destroy(finish);
                        return position;
                    }

                ```
    - emulator
    - csapp
</details>