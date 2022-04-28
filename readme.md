## 1 实验目的

构造LR(0)文法的Action表和GOTO表

## 2 实验要求

1. 用C++ 实现；

2. 编程构造如下LR(0)文法的Action表和GOTO表，文法G[S]： 

(0) S->E (1)E->aA (2) E->bB (3) A->cA (4) A->d (5) B->cB (6)B->d 

3. Action表和GOTO表输出到output.txt文件。 

4. 提供名为LR0.exe的执行文件实现上述功能，还需提供源文件以及名为设计说明.doc的说明文件。

## 3 结果展示

![在这里插入图片描述](https://img-blog.csdnimg.cn/ca071ee853f94d1583605345475d480f.png)

![](https://img-blog.csdnimg.cn/50a522ebd9364f1e964393f2de424cd1.png)


## 4 设计思路

### 4.1 总体思路

根据现有的文法，推导出如下图所示的DFA。

![在这里插入图片描述](https://img-blog.csdnimg.cn/52a69de5671f4b19a9a16a83c3de0f3e.png)


如何生成这样的DFA图，则使用数据结构的图结构，并使用邻接表的形式来构图。

在根据以上DFA图，顺序遍历各个项目,构造出LR(0)分析表，即ACTION和GOTO表



### 4.2详细思路

#### 4.2.1构建增广文法

本实验所提供的文法如下：

(0) S->E 

(1)E->aA 

(2) E->bB 

(3) A->cA 

(4) A->d 

(5) B->cB 

(6)B->d

已经满足增广文法的需要，直接在程序中使用

#### 4.2.2状态数据结构的确立

自定义数据结构`item`代表DFA图中的状态（如i<sub>0</sub>）,拥有如下成员函数

```c++
set<string> production; //状态包含的产生式
arc_item *next;         //指向连接的那个状态
int id;                 //状态号
int state = -1;         //状态,用于判断是否进行规约或转移
```

使用`set`数据结构来存储产生式，可以不用考虑生成的产生式重复的情况。

而`state`则用于生成action和GOTO表时，判断这个状态是否为移进还是规约还是直接接受。

state=0时，产生式为`S’->S.`型则置ACTION[k, #]为“接受”，简记为“acc”;

state>0时，产生式为`A->α.`型,对任何终结符a, 置ACTION[k, a]为“用产生式A->α进行归约”，简记为“r<sub>j</sub>>”,且state值对应j，用j号文法进行规约。

其余情况state=-1时，产生式形如Aα->.aβ，为移进情况。

#### 4.2.3状态的闭包计算

![在这里插入图片描述](https://img-blog.csdnimg.cn/963f14dcf54b493681ff7692c9ce2b73.png)


即为“.”后的字母如果为非终结符，则将该非终结符所能推导的产生式加入到该状态中，且将“.”加入到“->”后。

如初态为
I<sub>0</sub>：S->.E



闭包运算后为

I<sub>0</sub>:  S->.E
    E->.aA
    E->.bB



#### 4.2.4 构造LR(0)FSM的所有状态
![在这里插入图片描述](https://img-blog.csdnimg.cn/7b6496973d9c41878966eb16826b57e1.png)


即为

遍历当前的所有状态，再遍历当前状态I<sub>m</sub>的所有产生式，若“.”不在产生式的最后，

则将“.”向后移动一位，被跳过的字符为a，这样生成的一个新的产生式A，再生成一个新的状态I<sub>n</sub>，将新生成的产生式A加入到这个新的状态I<sub>n</sub>中，再对这个状态进行一次闭包计算。将这个I<sub>n</sub>加入到状态集中，且状态不能重复。这样完成后，在状态I<sub>m</sub>状态I<sub>n</sub>连接一条边a,即为状态I<sub>m</sub>接受a字符后转移到状态I<sub>n</sub>。

重复上述操作，直到状态机的数量不再增大。

代码实现

```c++
//状态集构造
void structure() 
{
    bool done = 0; //构造是否完成
    int i = 0;
    while (done == 0 || i <= items.size() - 1)
    {

        set<string>::iterator j;
        done = 1;
        for (j = items[i].production.begin(); j != items[i].production.end(); j++) // 产生式的遍历
        {
            string tmp = *j;
            int index = tmp.find('.');
            if (index == tmp.length() - 1) //'.'已经在最后了
                continue;

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
                arc_item *edge = new arc_item();
                edge->next = items[i].next;
                edge->id = items[n].id;
                edge->weight = trans;

                item &p_item = const_cast<item &>(items[i]); //由于在set中元素为const无法修改，需要使用const_cast来进行类型修改

                p_item.next = edge; //将边接入
            }
            else
            {
                items.push_back(new_item);
                done = 0; //有新的状态集，需要继续进行构造
                //使用插头法接入边节点
                arc_item *edge = new arc_item();
                edge->next = items[i].next;
                edge->id = new_item.id;
                edge->weight = trans;

                item &p_item = const_cast<item &>(items[i]); //由于在set中元素为const无法修改，需要使用const_cast来进行类型修改

                p_item.next = edge; //将边接入
            }
        }
        i++;
    }
```

构建完成后，DFA图即构造完成。

通过断点可查询到图的情况，如下图
![在这里插入图片描述](https://img-blog.csdnimg.cn/3913cad2f87a4ef4ab6fa9244f92f569.png)



得到的DFA图如下


![在这里插入图片描述](https://img-blog.csdnimg.cn/2f58e5a915734da0baf54b543c29ceeb.png)

#### 4.2.5绘制action goto表

构建一个二维数组，根据DFA图和各个状态state的值来确定内容，最后按格式输出即可。


## 5 算法亮点

​		由于在LR(0)FSM的构造中需要反复判断有没有重复，比如新生成的产生式是否与原来的重复，新生成的状态有没有重复，故使用`set`数据结构来储存，这样新产生式就可以直接加入，让`set`自行判断是否需要新增这个产生式。

​		采用了邻接表图的形式来构造DFA,十分直观便于理解和操作。
