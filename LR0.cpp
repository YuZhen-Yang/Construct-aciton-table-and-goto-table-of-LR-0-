#include <string>
#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <map>
#include <iomanip>
#include <set>
using namespace std;

template <typename T>

int length(T& arr)
{
    return sizeof(arr) / sizeof(arr[0]);
}

//输入文法
string G_input[] =
{
    "S->E",
    "E->aA",
    "E->bB",
    "A->cA",
    "A->d",
    "B->cB",
    "B->d" };

set<char> terminal;    //终结符
set<char> nonterminal; //非终结符

//获取终结符和非终结符
void get_term_and_nonterm()
{
    for (int i = 0; i < length(G_input); i++)
    {
        nonterminal.insert(G_input[i][0]);
    }
    for (int i = 0; i < length(G_input); i++)
    {
        string tmp = G_input[i].substr(3);
        for (int j = 0; j < tmp.length(); j++)
        {

            if (nonterminal.find(tmp[j]) == nonterminal.end()) //未找到
            {
                terminal.insert(tmp[j]);
            }
        }
    }
}

//加入“.”
void add_point(string& a)
{
    a.insert(3, 1, '.');
}
//移动“.”
void move_point(string& a)
{
    int index = a.find('.');

    a[index] = a[index + 1];
    a[index + 1] = '.';
}

//使用邻接表的图结构
int t_id = 0; //状态号

struct arc_item;
struct item //状态,相当于节点
{
    set<string> production; //状态包含的产生式
    arc_item* next;         //指向连接的那个状态
    int id;                 //状态号
    int state = -1;         //状态,用于判断是否进行规约或转移

    item(string p) //初始化函数
    {
        production.insert(p);
        id = t_id;
        t_id++;
        next = NULL;
    }
    void revise() //更新产生式
    {
        bool done = 0; //是否继续更新
        while (done == 0)
        {
            done = 1;
            set<string>::iterator i;
            int num = production.size();
            for (i = production.begin(); i != production.end(); i++) // 产生式的遍历
            {
                string tmp = *i;
                int index = tmp.find('.');
                if (index == tmp.length() - 1) //'.'已经在最后了
                {
                    for (int k = 0; k < length(G_input); k++)
                    {
                        if (tmp.substr(0, tmp.length() - 1) == G_input[k])
                        {
                            state = k;
                            break;
                        }
                    }
                    continue;
                }
                if (nonterminal.find(tmp[index + 1]) != nonterminal.end()) //'.'后是一个非终结符，进行闭包计算
                {
                    for (int g = 0; g < length(G_input); g++)
                    {
                        if (G_input[g][0] == tmp[index + 1])
                        {
                            string new_p = G_input[g];
                            add_point(new_p);
                            production.insert(new_p);
                        }
                    }
                }
            }
            if (production.size() > num) //个数有增加
                done = 0;                //需要继续更新
        }
    }

    bool operator==(const item W)
    {
        return production == W.production; //根据产生式判断是否相等
    }
    bool operator<(const item W) const
    {
        if (production == W.production)
            return false;
        else
            return id < W.id;
    }
};

struct arc_item //边节点
{
    int id; //状态号
    arc_item* next;
    char weight; //连接边的权重，即为状态转移的输入的值
};

struct fsm
{
    vector<item> items; //状态集

    fsm(string s)
    {
        string first = s;
        add_point(first);
        item item1 = item(first);
        item1.revise();
        items.push_back(item1);
    }

    //状态集构造
    void structure()
    {
        bool done = 0; //构造是否完成
        int i = 0;
        while (done == 0 || i <= items.size() - 1)
        {

            set<string>::iterator j= items[i].production.begin() ;
            done = 1;
            while (j != items[i].production.end()) // 产生式的遍历
            {
                string tmp = *j;
                int index = tmp.find('.');
                if (index == tmp.length() - 1) //'.'已经在最后了
                {   ++j;
                    continue;
                }
                char trans = tmp[index + 1]; //转移状态所需的字符
                move_point(tmp);
                item new_item = item(tmp);
                new_item.revise();

                bool same = false;
                int n; //用于标记已存在的状态
                for (int k = 0; k < items.size(); k++)
                {
                    if (new_item == items[k])
                    {
                        same = true;
                        n = k;
                        break;
                    }
                }
                if (same == true) //已经存在该状态集
                {
                    t_id--;
                    arc_item* edge = new arc_item();
                    edge->next = items[i].next;
                    edge->id = items[n].id;
                    edge->weight = trans;

                    item& p_item = const_cast<item&>(items[i]); //由于在set中元素为const无法修改，需要使用const_cast来进行类型修改

                    p_item.next = edge; //将边接入
                }
                else
                {
                    string tmp_find = *j;
                    items.push_back(new_item);
                    j=items[i].production.find(tmp_find);
                    done = 0; //有新的状态集，需要继续进行构造
                    //使用插头法接入边节点
                    arc_item* edge = new arc_item();
                    edge->next = items[i].next;
                    edge->id = new_item.id;
                    edge->weight = trans;

                    item& p_item = const_cast<item&>(items[i]); //由于在set中元素为const无法修改，需要使用const_cast来进行类型修改

                    p_item.next = edge; //将边接入
                }
                ++j;
            }
            i++;
        }
    }

    //绘制action goto表
    void show_table()
    {
        int num = items.size(); //状态个数
        int line = terminal.size() + nonterminal.size() + 1;
        vector<vector<string> > matrix(num, vector<string>(line));

        map<char, int> my_map;
        int m = 0;
        set<char>::iterator index;

        for (index = terminal.begin(); index != terminal.end(); index++)
        {
            my_map.insert(pair<char, int>(*index, m));
            m++;
        }
        my_map.insert(pair<char, int>('#', m));
        m++;
        for (index = nonterminal.begin(); index != nonterminal.end(); index++)
        {
            if (*index == 'S')
                continue;
            my_map.insert(pair<char, int>(*index, m));
            m++;
        }

        for (int i = 0; i < items.size(); i++)
        {
            if (items[i].state == 0)
            {
                matrix[i][my_map['#']] = "acc";
            }
            else if (items[i].state > 0)
            {
                for (int k = 0; k <= my_map['#']; k++)
                {
                    matrix[i][k] = "r" + to_string(items[i].state);
                }

            }
            else
            {
                arc_item* tmp = items[i].next;
                while (tmp != NULL)
                {
                    if (nonterminal.find(tmp->weight) == nonterminal.end())
                    {
                        matrix[i][my_map[tmp->weight]] = "S" + to_string(tmp->id);

                    }
                    else matrix[i][my_map[tmp->weight]] = to_string(tmp->id);
                    tmp = tmp->next;
                }
            }

        }

        //----------------------------------------  ----------------------------------------

        cout << setw(5) << "状态" << "|";
        cout << setw(25) << "ACTION|";
        cout << setw(15) << "GOTO" << endl;
        cout << setw(5) << "" << "|";
        cout << setw(5) << "a|" << setw(5) << "b|" << setw(5) << "c|" << setw(5) << "d|" << setw(5) << "#|"
            << setw(5) << "A|" << setw(5) << "B|" << setw(5) << "E|" << endl;
        for (int i = 0; i < num; i++)
        {
            cout << setw(5) << i << "|";
            for (int j = 0; j < line - 1; j++)
            {
                cout << setw(5) << matrix[i][j] + "|";
            }
            cout << endl;
        }

        ofstream out("output.txt");
        out << "LR(0)的分析表如下" << endl << endl;

        out << setw(5) << "状态" << "|";
        out << setw(25) << "ACTION|";
        out << setw(15) << "GOTO" << endl;
        out << setw(5) << "" << "|";
        out << setw(5) << "a|" << setw(5) << "b|" << setw(5) << "c|" << setw(5) << "d|" << setw(5) << "#|"
            << setw(5) << "A|" << setw(5) << "B|" << setw(5) << "E|" << endl;
        for (int i = 0; i < num; i++)
        {
            out << setw(5) << i << "|";
            for (int j = 0; j < line - 1; j++)
            {
                out << setw(5) << matrix[i][j] + "|";
            }
            out << endl;
        }



    }


};


int main()
{
    get_term_and_nonterm();
    string first = "S->E";


    fsm lr_0 = fsm(first);
    lr_0.structure();

    lr_0.show_table();



    system("pause");
}
