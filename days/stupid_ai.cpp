#include <cstdio>
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <vector>

using namespace std;

#define row 3
#define col 3
#define Computer -1
#define Player 1

class Board;
class Pair;
class TreeNode;
class Tree;

int go_first = 0;
int board[3][3];
int step = 0;

void welcome();
int ask();
void start(int first);
void Computer_first(TreeNode *);
void Player_first(TreeNode *);
void check();
int check_silence(Board b);
void Player_win();
void Computer_win();
void Nowinner();
void Show();
char n2c(int x);
int max(vector<int>);
int min(vector<int>);
Pair compare(TreeNode*, TreeNode*);


class Pair
{
public:
    int x;
    int y;

    Pair(int x = -1, int y = -1) { this->x = x; this->y = y; }
};


#if 0
class Board
{
public:
    int** b;
public:

    Board() {
        b = new int* [3];
        b[0] = new int[3];
        b[1] = new int[3];
        b[2] = new int[3];

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = 0;
    }

    Board(int bb[][3]){
        b = new int* [3];
        b[0] = new int[3];
        b[1] = new int[3];
        b[2] = new int[3];

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = bb[i][j];
    }

    Board(int** bb){
        b = new int* [3];
        b[0] = new int[3];
        b[1] = new int[3];
        b[2] = new int[3];

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = bb[i][j];
    }

    ~Board() {
        //delete b[0];
        //delete b[1];
        //delete b[2];
        //delete b;
        delete[] b;
    }

    Board(Board& B) {
        b = new int* [3];
        b[0] = new int[3];
        b[1] = new int[3];
        b[2] = new int[3];

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = B.b[i][j];
    }

    void set_borad(Pair p, int user) { b[p.x][p.y] = user; }

    vector<Pair> getBlank() {
        vector<Pair> res;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (!this->b[i][j]) {
                    Pair temp(i, j);
                    res.push_back(temp);
                }
            }
        }
        return res;
    }

    void show() {
        //system("cls");
        puts("---------------------");
        for (int i = 0; i < 3; i++) {
            putchar('|');
            for (int j = 0; j < 3; j++) {
                putchar(n2c(this->b[i][j]));
                putchar('|');
            }
            printf("\n");
        }
        puts("-----------------------");
    }
};
#endif

#if 1
class Board
{
public:
    int b[3][3];
public:

    Board() {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = 0;
    }

    Board(int bb[][3]) {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = bb[i][j];
    }

    Board(int** bb) {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = bb[i][j];
    }

    ~Board() {
        //delete b[0];
        //delete b[1];
        //delete b[2];
        //delete b;
        //delete[] b;
    }

    Board(Board& B) {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                b[i][j] = B.b[i][j];
    }

    void set_borad(Pair p, int user) { b[p.x][p.y] = user; }

    vector<Pair> getBlank() {
        vector<Pair> res;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (!this->b[i][j]) {
                    Pair temp(i, j);
                    res.push_back(temp);
                }
            }
        }
        return res;
    }

    void show() {
        //system("cls");
        puts("---------------------");
        for (int i = 0; i < 3; i++) {
            putchar('|');
            for (int j = 0; j < 3; j++) {
                putchar(n2c(this->b[i][j]));
                putchar('|');
            }
            printf("\n");
        }
        puts("-----------------------");
    }
};
#endif
class TreeNode
{
private:
    int level;
    int ceil;
    int floor;
    TreeNode* parent;
    vector<TreeNode*> childrens;
    Board* b;
public:
    TreeNode(Board bb, int l, int c = INT_MAX, int f = INT_MIN, TreeNode* p = NULL) {

        b = new Board(bb);

        level = l;
        ceil = c;
        floor = f; 
        parent = p;
        childrens = vector<TreeNode*>();
    }

    ~TreeNode() {
        delete b;
    }

    TreeNode* getParent() { return this->parent; }
    vector<TreeNode*> getChildren() { return this->childrens; }
    int getLevel() { return this->level; }
    void setLevel(int x) { this->level = x; }
    int getCeil() { return this->ceil; }
    void setCeil(int x) { this->ceil = x;}
    int getFloor() { return this->floor; }
    void setFloor(int x) { this->floor = x; }
    void setScore(int x){this->floor = this->ceil = x;}

    bool isExact() { return this->ceil == this->floor; }

    void setParent(TreeNode* p) { this->parent = p; }
    void addChild(TreeNode* c) { this->childrens.push_back(c); }

    Board* getBoard() { return this->b;}

};


class Tree {
public:
    TreeNode* root;
public:
    Tree() {
        Board b;
        root = new TreeNode(b, 0);
        buildTree(root,go_first);
        scoreTree(root,go_first);
    }

    Tree(TreeNode* root,int user) {
        buildTree(root, user);
        scoreTree(root, go_first);
    }

    void buildTree(TreeNode* root, int user) {
        if (check_silence(*(root->getBoard())) != 0) {
            return;     
        }
        vector<Pair> res = root->getBoard()->getBlank();
        int l = root->getLevel();
        for (Pair p : res) {
            Board temp = *(root->getBoard());
            temp.set_borad(p,user);
            TreeNode* c = new TreeNode(temp,l+1);
            c->setParent(root);
            root->addChild(c);
            buildTree(c, user * (-1));
        }
    }

#if 0
    void scoreTree(TreeNode* root, int user){
        int res = check_silence(*(root->getBoard()));
        if(res!=0){
            if(res == Computer){
                root->setScore(Computer);
                return;
            }else{
                root->setScore(Player);
                return;
            }
        }
        vector<TreeNode*> ch = root->getChildren();

        if (ch.empty()) {
            root->setScore(0);
            return;
        }

        vector<int> scores;
        for (TreeNode* c : ch) {
            if (c->getCeil() == c->getFloor()) {
                scores.push_back(c->getCeil());
            }
            else {
                scoreTree(c, user * -1);
                scores.push_back(c->getCeil());
            }
        }
        if (user == Computer) {
            root->setScore(min(scores));
        }
        else {
            root->setScore(max(scores));
        }
        return;
        
    }
#endif

#if 1
    static void scoreTree(TreeNode* root, int user) {

        int res = check_silence(*(root->getBoard()));

        if (res != 0) {     // 叶子节点
            if (res == Computer) {
                root->setScore(Computer);
                return;
            }
            else {
                root->setScore(Player);
                return;
            }
        }

        vector<TreeNode*> ch = root->getChildren();

        if (ch.empty()) {       // 叶子节点
            root->setScore(0);
            return;
        }

        if (user == Computer) // MinPlayer
        {
            for (TreeNode* c : ch) {
                if (c->getFloor() == INT_MIN) { scoreTree(c, user * -1); } // 如果子节点没有scored
                if (c->getFloor() < root->getCeil()) root->setScore(c->getFloor()); // 更新当前上界

                if (root->getParent()) {
                    if (root->getParent()->getFloor() > root->getCeil()) break;  // 剪枝
                }
            }
        }
        else {
            for (TreeNode* c : ch) {  // MaxPlayer
                if (c->getCeil() == INT_MAX) { scoreTree(c, user * -1); }
                if (c->getCeil() > root->getFloor()) root->setScore(c->getCeil());
                
                if (root->getParent()) {
                    if (root->getParent()->getCeil() < root->getFloor()) break;
                }
             
            }
        }
}
#endif
};

void test_show(TreeNode * root) {
    root->getBoard()->show();
    vector<TreeNode*> ch = root->getChildren();
    if (ch.empty()) return;
    test_show(ch[0]);
}


void test_tree() {
    int bb[][3] = { {1,-1,0},
                    {-1,1,-1},
                    {0,1,-1} };

    TreeNode* root = new TreeNode(bb, 7);
    Tree* tree = new Tree(root,Player);
    test_show(root);
}

int max(vector<int> nums) {
    int res = INT_MIN;
    for (int i : nums) {
        if (i > res){
            res = i;
        }
    }
    return res;
}

int min(vector<int> nums) {
    int res = INT_MAX;
    for (int i : nums) {
        if (i < res) {
            res = i;
        }
    }
    return res;
}

void welcome()
{
    puts("----Welcome------");
    return;
}

int ask()
{
    char buf[0x10];
    puts("Go first?[Y/n]");
    buf[0] = getchar();
    if (buf[0] == 'Y' || buf[0] == 'y' || buf[0] == '\n')
        return Player;
    return Computer;
}

void start(int first)
{
    //int bb[][3] = { {0,0,0},{1,-1,-1},{0,1,0} };
    int bb[][3] = { {0,0,0},{0,0,0},{0,0,0} };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = bb[i][j];
            if (bb[i][j]) step++;
        }
    }

    TreeNode* root = new TreeNode(bb, step);
    Tree* tree = new Tree(root, first);
    
    if (first == Computer) {
        while (1) {
            Computer_first(root);
        }
    }
    else {
        Player_first(root);
    }
    return;
}

int check_silence(Board board)
{   
    int b[3][3] = { 0 };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            b[i][j] = board.b[i][j];
        }
    }
    int win[9] = { 0 };
    win[1] = b[0][0] + b[0][1] + b[0][2];
    win[2] = b[1][0] + b[1][1] + b[1][2];
    win[3] = b[2][0] + b[2][1] + b[2][2];
    win[4] = b[0][0] + b[1][0] + b[2][0];
    win[5] = b[0][1] + b[1][1] + b[2][1];
    win[6] = b[0][2] + b[1][2] + b[2][2];
    win[7] = b[0][0] + b[1][1] + b[2][2];
    win[8] = b[0][2] + b[1][1] + b[2][0];

    for (int i = 0; i < 9; i++) {
        if (win[i] == 3) {
            return Player;
        }
        if (win[i] == -3) {
            return Computer;
        }
    }
    return 0;
}

void check()
{
    // check row
    int win[9] = { 0 };
    win[1] = board[0][0] + board[0][1] + board[0][2];
    win[2] = board[1][0] + board[1][1] + board[1][2];
    win[3] = board[2][0] + board[2][1] + board[2][2];
    win[4] = board[0][0] + board[1][0] + board[2][0];
    win[5] = board[0][1] + board[1][1] + board[2][1];
    win[6] = board[0][2] + board[1][2] + board[2][2];
    win[7] = board[0][0] + board[1][1] + board[2][2];
    win[8] = board[0][2] + board[1][1] + board[2][0];

    for (int i = 0; i < 9; i++) {
        if (win[i] == 3) {
            Player_win();
        }
        if (win[i] == -3) {
            Computer_win();
        }
    }

    if (step == 9)
        Nowinner();
}

void Player_win() {
    system("cls");
    puts("Your Win!");
    exit(0);
}

void Computer_win() {
    system("cls");
    puts("Your lose!");
    exit(0);
}

void Nowinner() {
    system("cls");
    puts("No winner!");
    exit(0);
}

char n2c(int x) {
    if (x == 1) return 'X';
    if (x == -1) return 'O';
    else return ' ';
}

void Show()
{
    system("cls");
    puts("---------------------");
    for (int i = 0; i < 3; i++) {
        putchar('|');
        for (int j = 0; j < 3; j++) {
            putchar(n2c(board[i][j]));
            putchar('|');
        }
        printf("\n");
    }
    puts("-----------------------");
}

Pair compare(TreeNode* root, TreeNode* child) {
    Board* root_b = root->getBoard();
    Board* child_b = child->getBoard();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (root_b->b[i][j] == 0 && child_b->b[i][j] != 0) {
                return Pair(i, j);
            }
        }
    }
    return NULL;
}

Pair next_step(TreeNode* root, int user)
{
    vector<TreeNode*> ch = root->getChildren();
    for (TreeNode* c : ch) {
        if (c->getCeil() == root->getCeil()) {
            Pair ans = compare(root, c);
            return ans;
        }
    }
    return NULL;
}

TreeNode* follow(TreeNode* root, Pair pair) {
    vector<TreeNode*> ch = root->getChildren();
    for (TreeNode* c : ch) {
        Board* cb = c->getBoard();
        if (cb->b[pair.x][pair.y] != 0) {
            if (c->getCeil() != c->getFloor()) {
                Tree* tree = new Tree(c, (cb->b[pair.x][pair.y])*-1);
            }
            return c;
        }
    }
    return NULL;
}

void Computer_first(TreeNode * root) {
    Pair pair;
    int x;
    int y;
    while (1) {
        Show();
        pair = next_step(root,Computer);
        assert(!board[pair.x][pair.y]);
        board[pair.x][pair.y] = Computer;
        root = follow(root, pair);
        step++;
        check();

        Show();
        printf("Your step: ");
        cin >> x >> y;
        board[x][y] = Player;
        pair.x = x;
        pair.y = y;
        assert(!board[pair.x][pair.y]);
        root = follow(root, pair);
        step++;
        check();
    }
}

void Player_first(TreeNode * root) {
    Pair pair;
    int x;
    int y;
    while (1) {
        Show();
        printf("Your step: ");
        cin >> x >> y;
        board[x][y] = Player;
        pair.x = x;
        pair.y = y;
        assert(!board[pair.x][pair.y]);
        root = follow(root, pair);
        step++;
        check();

        Show();
        pair = next_step(root,Computer);
        assert(!board[pair.x][pair.y]);
        board[pair.x][pair.y] = Computer;
        root = follow(root, pair);
        step++;
        check();
    }
}

int main()
{
    welcome();
    go_first = ask();
    start(go_first);
    return 0;
}