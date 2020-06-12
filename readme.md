# 天问之路

此项目用以记录跟随sakura师傅学习的经历，和平时自己学习的点滴。项目不会包括具体的知识，只是一个类似周记的记录，用来给自己看。

---
[第一周](#week1)<br />
[第二周](#week2)<br />
[第三周](#week3)<br />
[第四周](#week4)<br />

### <h3 id="week1">5.17-5.22</h3>
- STL<br />
目标：熟悉c++语法，了解STL标准库的实现。<br />
进度：allocator-> iterator-> basefunction -> list -> vector->hashtable<br />
收获：<br />
    - <a herf = "https://www.xuebuyuan.com/1919018.html">traits的使用</a><br />
    - <a herf = "https://blog.csdn.net/qq_25343557/article/details/89110319">自平衡树算法（AVL）</a><br />
    - 红黑树（TODO）
    - vector erase可能造成的漏洞<br>
    
        ```
                iterator erase(iterator position) {
                    if (position + 1 != end())
                    {
                    copy(position + 1, finish, position);
                    }
                    --finish;
                    destroy(finish);
                    return position;
                }
        ```
        问题出在，如果vector中存放的是指针的话，copy函数只进行了浅拷贝，导致末尾的指针在vector中超过一次出现，可能会造成UAF。<br />
        poc
        ```
        #include <iostream>
        #include <vector>
        #include <cstdlib>
        using namespace std;

        class mypair{
        public:
            char * ptr;
            mypair(){ ptr = (char*)malloc(0x40);}
            ~mypair(){ cout<<"destory this"<<endl;free(ptr);} 
        };

        int main(){
            vector<mypair> vec;
            mypair*  ptr;

            for(int i = 0; i < 4; i++){
                ptr = new mypair();
                vec.push_back(*ptr);
            }

            vector<mypair>::iterator it = vec.begin();
            it = vec.begin();
            vec.erase(it);
            it = vec.begin();
            vec.erase(it);
            return 0;
        }
        ```      

### <h3 id="week2">5.22-5.29</h3>
这周忙了一些实验和大作业，做的工作稍微少了点（x

- cs143
    - 进度： 第三周 18节
    - 收获：了解了词法分析的基本方法，自动机的原理， NFA->DFA

- csapp
    进度：1-4章 7章 8章（部分）<br />
    有收获的知识点：
    - 数据的表示(x)翻译(√)
    - 多级流水设计
    - 静态链接和动态链接，特别是静态链接
    - 异常（TODO）

- emulator<br />
    qbx是一个用宏编程写的简单的emulator。<br />
    我的主要工作就是读懂现有的代码，并且为它补全branch和运算的指令。<br />
    项目地址：https://github.com/Mosk0ng/qbxvm <br />
    资料地址：https://gpfault.net/posts/asm-tut-2.txt.html <br />

- 测试：<br>
    [题目](./test/test1.md)<br />
    [解答](./solution/test1/vector.cpp)</a>

### <h3 id="week3">5.29-6.5</h3>

开始做compiler的部分，主要是看网课<br />
[哈工大视频](https://www.icourse163.org/learn/HIT-1002123007?tid=1450215473#/learn/content?type=detail&id=1214538555&cid=1218322632&replay=true)

一直看完了语法分析的部分，然后写(抄)了斯坦福cs143的lab前两个任务，参考的是这个项目<br />
[参考项目](https://github.com/skyzluo/CS143-Compilers-Stanford/blob/master)

对这个东西还是不是很熟悉，基本就是照抄了，边抄便理解。

看了两本书，编译原理（龙书）和flex与bison，都是浅浅的看了，下一周会重点看一下。

这周由于有考试和论文，工作少了一些,有两天的笔记。
看了一个`avast！`的洞，具体在6.3的笔记里

[6.2笔记](./days/2020_6_2.md) <br/>
[6.3笔记](./days/2020_6_3.md) <br/>

### <h3 id="week4">6.6-6.12</h3>

细看龙书和flex和bison，分析flex和bison生成的代码，写了一些test的代码。
分析报告<br />
[flex_bison生成代码的分析报告(初步)](./days/2020_6_11.md)

大概的分析了逻辑，具体的数据结构方面的深入分析，还得补完龙书。

`codebase`不太行，听`sakura`师傅的，先去补一下`scip`，`compiler`先放一放




