#include <stdio.h>

#define N 10

int parent[N];
int size[N];

// 初始：每个节点的父节点指向自身，相对于每个节点都是一个独立的集合
void makeSet() 
{
    for (int i = 0; i < N; i++) {
        parent[i] = i;   // 节点i的父节点
        size[i] = 1;     // 节点 i 为根的树的规模
    } 
}


// 路径压缩，以缩短后续查找路径，提高查找效率
// 找到集合的根节点，每个集合都只有一个根节点
int find(int x) 
{
	// 找到根节点
    int root = x;
    while (root != parent[root]) {
        root = parent[root];  
    }

    // 路径压缩优化，将 x 到根节点路径上的所有节点直接连接到根节点
    while (x != root) {
        int temp = parent[x];
        parent[x] = root;
        x = temp;
    }
    return root; 
}


// 小的合并到 大的上
void unionSets(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);
    if (rootX != rootY) {
        if (size[rootX] < size[rootY]) {
            parent[rootX] = rootY;
            size[rootY] += size[rootX];
        } else {
            parent[rootY] = rootX;
            size[rootX] += size[rootY];
        }
    }
}

int main() 
{
    makeSet();

    unionSets(0, 1);
    unionSets(1, 2);
    unionSets(0, 3);
    unionSets(4, 5);
    unionSets(2, 5);
    unionSets(7, 8);
    unionSets(8, 9);

    for (int i = 0; i < N; i++) {
        printf("Element %d: Parent - %d, Size - %d\n", i, parent[i], size[i]);
    }

    return 0;
}